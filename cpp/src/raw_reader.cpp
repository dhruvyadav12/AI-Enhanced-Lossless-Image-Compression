#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>

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
// PREDICTOR
// ==================================================

int predict(
    const vector<unsigned char>& pixels,
    int index
) {

    if (index == 0) {
        return 0;
    }

    return pixels[index - 1];
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
// GENERATE HUFFMAN CODES
// ==================================================

void generateCodes(
    HuffmanNode* node,
    string code,
    unordered_map<int, string>& codes
) {

    if (node == nullptr) {
        return;
    }

    // Leaf node
    if (node->left == nullptr &&
        node->right == nullptr) {

        codes[node->value] = code;

        return;
    }

    if (node->left != nullptr) {

        generateCodes(
            node->left,
            code + "0",
            codes
        );
    }

    if (node->right != nullptr) {

        generateCodes(
            node->right,
            code + "1",
            codes
        );
    }
}


// ==================================================
// PACK BITS INTO BYTES
// ==================================================

vector<unsigned char> packBits(
    const string& bits,
    int& validBits
) {

    validBits = bits.size();

    vector<unsigned char> bytes;

    for (size_t i = 0; i < bits.size(); i += 8) {

        unsigned char byte = 0;

        for (int j = 0; j < 8; j++) {

            byte <<= 1;

            if (i + j < bits.size() &&
                bits[i + j] == '1') {

                byte |= 1;
            }
        }

        bytes.push_back(byte);
    }

    return bytes;
}


// ==================================================
// MAIN
// ==================================================

int main() {

    // ------------------------------------------------
    // 1. READ RAW IMAGE
    // ------------------------------------------------

    string filePath =
        "../../data/input/image.raw";

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


    cout << "Raw image loaded successfully!"
         << endl;

    cout << "Total pixels: "
         << pixels.size()
         << endl;


    // ------------------------------------------------
    // 2. PREDICTION + RESIDUALS
    // ------------------------------------------------

    vector<int> residuals;

    residuals.reserve(
        pixels.size()
    );


    for (size_t i = 0;
         i < pixels.size();
         i++) {

        int prediction =
            predict(
                pixels,
                i
            );

        int residual =
            static_cast<int>(pixels[i])
            - prediction;

        residuals.push_back(
            residual
        );
    }


    cout << "Total residuals: "
         << residuals.size()
         << endl;


    // ------------------------------------------------
    // 3. ZIGZAG
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


    cout << "Total ZigZag values: "
         << zigzagValues.size()
         << endl;


    // ------------------------------------------------
    // 4. FREQUENCY TABLE
    // ------------------------------------------------

    unordered_map<int, int> frequency;


    for (int value : zigzagValues) {

        frequency[value]++;
    }


    cout << "Unique ZigZag values: "
         << frequency.size()
         << endl;


    // ------------------------------------------------
    // 5. HUFFMAN PRIORITY QUEUE
    // ------------------------------------------------

    priority_queue<
        HuffmanNode*,
        vector<HuffmanNode*>,
        Compare
    > pq;


    for (auto entry : frequency) {

        HuffmanNode* node =
            new HuffmanNode(
                entry.first,
                entry.second
            );

        pq.push(node);
    }


    cout << "Huffman nodes created: "
         << pq.size()
         << endl;


    // ------------------------------------------------
    // 6. BUILD HUFFMAN TREE
    // ------------------------------------------------

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


    HuffmanNode* root =
        pq.top();


    cout << "Huffman tree built!"
         << endl;

    cout << "Root frequency: "
         << root->frequency
         << endl;


    // ------------------------------------------------
    // 7. GENERATE CODES
    // ------------------------------------------------

    unordered_map<int, string> codes;


    generateCodes(
        root,
        "",
        codes
    );


    cout << "Huffman codes generated: "
         << codes.size()
         << endl;


    // ------------------------------------------------
    // 8. CREATE ENCODED BITSTREAM
    // ------------------------------------------------

    string encodedBits;

    encodedBits.reserve(
        7002494
    );


    for (int value : zigzagValues) {

        encodedBits +=
            codes[value];
    }


    cout << "Encoded bitstream created."
         << endl;

    cout << "Total encoded bits: "
         << encodedBits.size()
         << endl;


    // ------------------------------------------------
    // 9. PACK BITS INTO BYTES
    // ------------------------------------------------

    int validBits = 0;


    vector<unsigned char> compressedBytes =
        packBits(
            encodedBits,
            validBits
        );


    cout << "Valid bits: "
         << validBits
         << endl;

    cout << "Compressed bytes: "
         << compressedBytes.size()
         << endl;


    // ------------------------------------------------
    // 10. WRITE COMPRESSED FILE
    // ------------------------------------------------

    string outputPath =
        "../../data/output/compressed.aic";


    ofstream outputFile(
        outputPath,
        ios::binary
    );


    if (!outputFile) {

        cout << "ERROR: Could not create "
             << "compressed.aic"
             << endl;

        return 1;
    }


    outputFile.write(
        reinterpret_cast<const char*>(
            compressedBytes.data()
        ),
        compressedBytes.size()
    );


    outputFile.close();


    cout << "Compressed file created!"
         << endl;

    cout << "Output: "
         << outputPath
         << endl;


    // ------------------------------------------------
    // 11. SIZE INFORMATION
    // ------------------------------------------------

    cout << endl;

    cout << "========== COMPRESSION RESULTS =========="
         << endl;

    cout << "Original raw size: "
         << pixels.size()
         << " bytes"
         << endl;

    cout << "Compressed data size: "
         << compressedBytes.size()
         << " bytes"
         << endl;


    double ratio =
        static_cast<double>(
            pixels.size()
        )
        /
        compressedBytes.size();


    cout << "Compression ratio: "
         << ratio
         << "x"
         << endl;


    double reduction =
        100.0 *
        (
            1.0 -
            static_cast<double>(
                compressedBytes.size()
            )
            /
            pixels.size()
        );


    cout << "Size reduction: "
         << reduction
         << "%"
         << endl;


    cout << "=========================================="
         << endl;


    return 0;
}