#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>

using namespace std;


// ==================================================
// HUFFMAN NODE
// ==================================================

struct HuffmanNode {

    int value;
    int frequency;

    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(int value, int frequency) {
        this->value = value;
        this->frequency = frequency;
        left = nullptr;
        right = nullptr;
    }
};


// ==================================================
// HUFFMAN COMPARATOR
// ==================================================

struct Compare {

    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->frequency > b->frequency;
    }
};


// ==================================================
// PAETH PREDICTOR
// ==================================================

int paethPredictor(
    const vector<unsigned char>& pixels,
    int index,
    int width
) {

    // First pixel
    if (index == 0) {
        return 0;
    }

    int left = pixels[index - 1];

    // First row
    if (index < width) {
        return left;
    }

    int top = pixels[index - width];

    int topLeft = pixels[index - width - 1];

    int p = left + top - topLeft;

    int pa = abs(p - left);
    int pb = abs(p - top);
    int pc = abs(p - topLeft);

    if (pa <= pb && pa <= pc) {
        return left;
    }

    if (pb <= pc) {
        return top;
    }

    return topLeft;
}


// ==================================================
// ZIGZAG
// ==================================================

int zigzagEncode(int value) {

    if (value >= 0) {
        return 2 * value;
    }

    return -2 * value - 1;
}


// ==================================================
// MAIN
// ==================================================

int main() {

    int width = 1086;
    int height = 1448;

    string filePath =
        "../../data/input/image.raw";


    // ------------------------------------------------
    // READ IMAGE
    // ------------------------------------------------

    ifstream inputFile(
        filePath,
        ios::binary
    );


    if (!inputFile) {

        cout << "ERROR: Could not open image.raw"
             << endl;

        return 1;
    }


    vector<unsigned char> pixels(
        (istreambuf_iterator<char>(inputFile)),
        istreambuf_iterator<char>()
    );


    inputFile.close();


    cout << "Image loaded successfully!" << endl;

    cout << "Pixels: "
         << pixels.size()
         << endl;


    // ------------------------------------------------
    // PREDICTION + RESIDUAL
    // ------------------------------------------------

    vector<int> residuals;

    residuals.reserve(
        pixels.size()
    );


    for (int i = 0;
         i < pixels.size();
         i++) {

        int prediction =
            paethPredictor(
                pixels,
                i,
                width
            );


        int residual =
            static_cast<int>(pixels[i])
            - prediction;


        residuals.push_back(
            residual
        );
    }


    // ------------------------------------------------
    // ZIGZAG
    // ------------------------------------------------

    vector<int> zigzagValues;

    zigzagValues.reserve(
        residuals.size()
    );


    for (int residual : residuals) {

        zigzagValues.push_back(
            zigzagEncode(residual)
        );
    }


    // ------------------------------------------------
    // FREQUENCY TABLE
    // ------------------------------------------------

    unordered_map<int, int> frequency;


    for (int value : zigzagValues) {
        frequency[value]++;
    }


    cout << "Unique symbols: "
         << frequency.size()
         << endl;


    // ------------------------------------------------
    // HUFFMAN TREE
    // ------------------------------------------------

    priority_queue<
        HuffmanNode*,
        vector<HuffmanNode*>,
        Compare
    > pq;


    for (auto entry : frequency) {

        pq.push(
            new HuffmanNode(
                entry.first,
                entry.second
            )
        );
    }


    while (pq.size() > 1) {

        HuffmanNode* left =
            pq.top();

        pq.pop();


        HuffmanNode* right =
            pq.top();

        pq.pop();


        HuffmanNode* parent =
            new HuffmanNode(
                -1,
                left->frequency +
                right->frequency
            );


        parent->left = left;
        parent->right = right;

        pq.push(parent);
    }


    HuffmanNode* root = pq.top();


    // ------------------------------------------------
    // CODE LENGTHS
    // ------------------------------------------------

    unordered_map<int, int> codeLengths;

    vector<pair<HuffmanNode*, int>> stack;

    stack.push_back(
        {root, 0}
    );


    while (!stack.empty()) {

        auto current =
            stack.back();

        stack.pop_back();


        HuffmanNode* node =
            current.first;

        int depth =
            current.second;


        if (node->left == nullptr &&
            node->right == nullptr) {

            codeLengths[node->value] =
                depth;

            continue;
        }


        if (node->left != nullptr) {

            stack.push_back(
                {node->left, depth + 1}
            );
        }


        if (node->right != nullptr) {

            stack.push_back(
                {node->right, depth + 1}
            );
        }
    }


    // ------------------------------------------------
    // COMPRESSED SIZE
    // ------------------------------------------------

    long long totalBits = 0;


    for (auto entry : frequency) {

        int value =
            entry.first;

        long long freq =
            entry.second;

        int codeLength =
            codeLengths[value];


        totalBits +=
            freq * codeLength;
    }


    long long compressedBytes =
        (totalBits + 7) / 8;


    // ------------------------------------------------
    // RESULTS
    // ------------------------------------------------

    cout << endl;

    cout << "========== PAETH PREDICTOR =========="
         << endl;

    cout << "Original raw size: "
         << pixels.size()
         << " bytes"
         << endl;

    cout << "Total compressed bits: "
         << totalBits
         << endl;

    cout << "Compressed payload: "
         << compressedBytes
         << " bytes"
         << endl;


    double ratio =
        static_cast<double>(
            pixels.size()
        )
        /
        compressedBytes;


    cout << "Compression ratio: "
         << ratio
         << "x"
         << endl;


    double reduction =
        100.0 *
        (
            1.0 -
            static_cast<double>(
                compressedBytes
            )
            /
            pixels.size()
        );


    cout << "Size reduction: "
         << reduction
         << "%"
         << endl;


    cout << "====================================="
         << endl;


    return 0;
}