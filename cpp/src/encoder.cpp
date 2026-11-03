#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <cstdint>

#include "../include/aic_format.h"
#include "../include/huffman_format.h"

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
// AVERAGE PREDICTOR
// ==================================================

int averagePredictor(
    const vector<unsigned char>& pixels,
    int index,
    int width
) {

    if (index == 0) {
        return 0;
    }

    int left =
        pixels[index - 1];

    // First row
    if (index < width) {
        return left;
    }

    int top =
        pixels[index - width];

    return (left + top) / 2;
}


// ==================================================
// ZIGZAG ENCODING
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
// PACK BITS
// ==================================================

vector<unsigned char> packBits(
    const string& bits,
    uint64_t& validBits
) {

    validBits =
        static_cast<uint64_t>(
            bits.size()
        );

    vector<unsigned char> bytes;

    bytes.reserve(
        (bits.size() + 7) / 8
    );


    for (size_t i = 0;
         i < bits.size();
         i += 8) {

        unsigned char byte = 0;


        for (int j = 0;
             j < 8;
             j++) {

            byte <<= 1;


            if (i + j < bits.size() &&
                bits[i + j] == '1') {

                byte |= 1;
            }
        }


        bytes.push_back(
            byte
        );
    }


    return bytes;
}


// ==================================================
// FREE HUFFMAN TREE
// ==================================================

void deleteTree(
    HuffmanNode* node
) {

    if (node == nullptr) {
        return;
    }

    deleteTree(node->left);

    deleteTree(node->right);

    delete node;
}


// ==================================================
// MAIN
// ==================================================

int main(int argc, char* argv[]) {
    // ==================================================
// READ IMAGE DIMENSIONS
// ==================================================

if (argc != 3) {

    cout << "Usage: encoder.exe <width> <height>"
         << endl;

    return 1;
}

const uint32_t WIDTH =
    static_cast<uint32_t>(
        stoi(argv[1])
    );

const uint32_t HEIGHT =
    static_cast<uint32_t>(
        stoi(argv[2])
    );

const uint8_t CHANNELS = 1;

// Average Predictor
const uint8_t PREDICTOR = 1;

    // ==================================================
    // IMAGE INFORMATION
    // ==================================================


    // ==================================================
    // PATHS
    // ==================================================

    string inputPath =
        "../../data/input/image.raw";

    string outputPath =
        "../../data/output/compressed.aic";


    // ==================================================
    // READ RAW IMAGE
    // ==================================================

    ifstream inputFile(
        inputPath,
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


    cout << "========================================"
         << endl;

    cout << "        AIC LOSSLESS ENCODER"
         << endl;

    cout << "========================================"
         << endl;


    cout << "Image loaded."
         << endl;

    cout << "Width: "
         << WIDTH
         << endl;

    cout << "Height: "
         << HEIGHT
         << endl;

    cout << "Pixels: "
         << pixels.size()
         << endl;


    // ==================================================
    // CHECK IMAGE SIZE
    // ==================================================

    uint64_t expectedPixels =
        static_cast<uint64_t>(WIDTH)
        *
        static_cast<uint64_t>(HEIGHT);


    if (pixels.size() != expectedPixels) {

        cout << "ERROR: Image size does not match"
             << endl;

        return 1;
    }


    // ==================================================
    // PREDICTION + ZIGZAG
    // ==================================================

    vector<int> zigzagValues;

    zigzagValues.reserve(
        pixels.size()
    );


    for (size_t i = 0;
         i < pixels.size();
         i++) {

        int prediction =
            averagePredictor(
                pixels,
                static_cast<int>(i),
                WIDTH
            );


        int residual =
            static_cast<int>(
                pixels[i]
            )
            -
            prediction;


        int zigzag =
            zigzagEncode(
                residual
            );


        zigzagValues.push_back(
            zigzag
        );
    }


    cout << "Prediction + ZigZag complete."
         << endl;


    // ==================================================
    // FREQUENCY TABLE
    // ==================================================

    unordered_map<int, int> frequency;


    for (int value : zigzagValues) {

        frequency[value]++;
    }


    cout << "Unique Huffman symbols: "
         << frequency.size()
         << endl;


    // ==================================================
    // BUILD HUFFMAN TREE
    // ==================================================

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


    if (pq.empty()) {

        cout << "ERROR: No symbols found."
             << endl;

        return 1;
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


    HuffmanNode* root =
        pq.top();


    cout << "Huffman tree built."
         << endl;


    // ==================================================
    // GENERATE CODES
    // ==================================================

    unordered_map<int, string> codes;


    generateCodes(
        root,
        "",
        codes
    );


    cout << "Huffman codes generated: "
         << codes.size()
         << endl;


    // ==================================================
    // ENCODE BITSTREAM
    // ==================================================

    string encodedBits;


    encodedBits.reserve(
        6500000
    );


    for (int value : zigzagValues) {

        encodedBits +=
            codes[value];
    }


    cout << "Encoded bits: "
         << encodedBits.size()
         << endl;


    // ==================================================
    // PACK INTO BYTES
    // ==================================================

    uint64_t validBits = 0;


    vector<unsigned char> compressedBytes =
        packBits(
            encodedBits,
            validBits
        );


    cout << "Compressed payload: "
         << compressedBytes.size()
         << " bytes"
         << endl;


    // ==================================================
    // CREATE AIC HEADER
    // ==================================================

    AICHeader header;


    header.magic[0] = 'A';

    header.magic[1] = 'I';

    header.magic[2] = 'C';

    header.magic[3] = '1';


    header.version = 1;


    header.width =
        WIDTH;

    header.height =
        HEIGHT;


    header.channels =
        CHANNELS;


    header.predictor =
        PREDICTOR;


    header.validBits =
        validBits;


    header.symbolCount =
        static_cast<uint32_t>(
            frequency.size()
        );


    // ==================================================
    // WRITE AIC FILE
    // ==================================================

    ofstream outputFile(
        outputPath,
        ios::binary
    );


    if (!outputFile) {

        cout << "ERROR: Could not create "
             << "compressed.aic"
             << endl;

        deleteTree(root);

        return 1;
    }


    // --------------------------------------------------
    // HEADER
    // --------------------------------------------------

    outputFile.write(
        reinterpret_cast<const char*>(
            &header
        ),
        sizeof(AICHeader)
    );


    // --------------------------------------------------
    // HUFFMAN TABLE
    // --------------------------------------------------

    for (auto entry : frequency) {

        HuffmanEntry huffmanEntry;


        huffmanEntry.symbol =
            entry.first;


        huffmanEntry.frequency =
            entry.second;


        outputFile.write(
            reinterpret_cast<const char*>(
                &huffmanEntry
            ),
            sizeof(HuffmanEntry)
        );
    }


    // --------------------------------------------------
    // COMPRESSED DATA
    // --------------------------------------------------

    outputFile.write(
        reinterpret_cast<const char*>(
            compressedBytes.data()
        ),
        compressedBytes.size()
    );


    outputFile.close();


    // ==================================================
    // FINAL FILE SIZE
    // ==================================================

    ifstream finalFile(
        outputPath,
        ios::binary
    );


    finalFile.seekg(
        0,
        ios::end
    );


    uint64_t finalSize =
        finalFile.tellg();


    finalFile.close();


    // ==================================================
    // RESULTS
    // ==================================================

    cout << endl;

    cout << "========================================"
         << endl;

    cout << "          COMPRESSION RESULTS"
         << endl;

    cout << "========================================"
         << endl;


    cout << "Original size: "
         << pixels.size()
         << " bytes"
         << endl;


    cout << "Huffman payload: "
         << compressedBytes.size()
         << " bytes"
         << endl;


    cout << "Final AIC file: "
         << finalSize
         << " bytes"
         << endl;


    cout << "Header size: "
         << sizeof(AICHeader)
         << " bytes"
         << endl;


    cout << "Huffman symbols: "
         << frequency.size()
         << endl;


    cout << "Valid bits: "
         << validBits
         << endl;


    double ratio =
        static_cast<double>(
            pixels.size()
        )
        /
        static_cast<double>(
            finalSize
        );


    double reduction =
        100.0 *
        (
            1.0 -
            static_cast<double>(
                finalSize
            )
            /
            static_cast<double>(
                pixels.size()
            )
        );


    cout << "Compression ratio: "
         << ratio
         << "x"
         << endl;


    cout << "Size reduction: "
         << reduction
         << "%"
         << endl;


    cout << "========================================"
         << endl;


    cout << endl;

    cout << "AIC FILE CREATED SUCCESSFULLY!"
         << endl;

    cout << outputPath
         << endl;


    // ==================================================
    // CLEANUP
    // ==================================================

    deleteTree(root);


    return 0;
}