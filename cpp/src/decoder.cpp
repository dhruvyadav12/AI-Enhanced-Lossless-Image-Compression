#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cstdint>
#include <string>

#include "../include/aic_format.h"
#include "../include/huffman_format.h"

using namespace std;


// ==================================================
// HUFFMAN NODE
// ==================================================

struct HuffmanNode {

    int value;
    uint32_t frequency;

    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(
        int value,
        uint32_t frequency
    ) {
        this->value = value;
        this->frequency = frequency;

        left = nullptr;
        right = nullptr;
    }
};


// ==================================================
// COMPARATOR
// ==================================================

struct Compare {

    bool operator()(
        HuffmanNode* a,
        HuffmanNode* b
    ) {

        return a->frequency >
               b->frequency;
    }
};


// ==================================================
// BUILD HUFFMAN TREE
// ==================================================

HuffmanNode* buildHuffmanTree(
    const vector<HuffmanEntry>& entries
) {

    priority_queue<
        HuffmanNode*,
        vector<HuffmanNode*>,
        Compare
    > pq;


    for (const auto& entry : entries) {

        pq.push(
            new HuffmanNode(
                entry.symbol,
                entry.frequency
            )
        );
    }


    if (pq.empty()) {
        return nullptr;
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


    return pq.top();
}


// ==================================================
// FREE TREE
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
// ZIGZAG DECODING
// ==================================================

int zigzagDecode(
    int value
) {

    if (value % 2 == 0) {

        return value / 2;
    }

    return -(value / 2) - 1;
}


// ==================================================
// MAIN
// ==================================================

int main() {

    string inputPath =
        "../../data/output/compressed.aic";


    string outputRawPath =
        "../../data/output/reconstructed.raw";


    // ==================================================
    // 1. OPEN AIC FILE
    // ==================================================

    ifstream inputFile(
        inputPath,
        ios::binary
    );


    if (!inputFile) {

        cout << "ERROR: Could not open "
             << "compressed.aic"
             << endl;

        return 1;
    }


    cout << "AIC file opened."
         << endl;


    // ==================================================
    // 2. READ HEADER
    // ==================================================

    AICHeader header;


    inputFile.read(
        reinterpret_cast<char*>(
            &header
        ),
        sizeof(AICHeader)
    );


    if (!inputFile) {

        cout << "ERROR: Could not read header."
             << endl;

        return 1;
    }


    // ==================================================
    // 3. VERIFY MAGIC
    // ==================================================

    if (header.magic[0] != 'A' ||
        header.magic[1] != 'I' ||
        header.magic[2] != 'C' ||
        header.magic[3] != '1') {

        cout << "ERROR: Invalid AIC file."
             << endl;

        return 1;
    }


    cout << endl;

    cout << "========== AIC HEADER =========="
         << endl;


    cout << "Magic: "
         << header.magic[0]
         << header.magic[1]
         << header.magic[2]
         << header.magic[3]
         << endl;


    cout << "Version: "
         << static_cast<int>(
                header.version
            )
         << endl;


    cout << "Width: "
         << header.width
         << endl;


    cout << "Height: "
         << header.height
         << endl;


    cout << "Channels: "
         << static_cast<int>(
                header.channels
            )
         << endl;


    cout << "Predictor: "
         << static_cast<int>(
                header.predictor
            )
         << endl;


    cout << "Valid bits: "
         << header.validBits
         << endl;


    cout << "Huffman symbols: "
         << header.symbolCount
         << endl;


    cout << "================================"
         << endl;


    // ==================================================
    // 4. READ HUFFMAN TABLE
    // ==================================================

    vector<HuffmanEntry> entries;

    entries.reserve(
        header.symbolCount
    );


    for (
        uint32_t i = 0;
        i < header.symbolCount;
        i++
    ) {

        HuffmanEntry entry;


        inputFile.read(
            reinterpret_cast<char*>(
                &entry
            ),
            sizeof(HuffmanEntry)
        );


        if (!inputFile) {

            cout << "ERROR: Could not read "
                 << "Huffman table."
                 << endl;

            return 1;
        }


        entries.push_back(
            entry
        );
    }


    cout << "Huffman table loaded."
         << endl;


    // ==================================================
    // 5. BUILD HUFFMAN TREE
    // ==================================================

    HuffmanNode* root =
        buildHuffmanTree(
            entries
        );


    if (root == nullptr) {

        cout << "ERROR: Could not build "
             << "Huffman tree."
             << endl;

        return 1;
    }


    cout << "Huffman tree rebuilt."
         << endl;


    // ==================================================
    // 6. READ COMPRESSED PAYLOAD
    // ==================================================

    vector<unsigned char> compressedBytes(
        (
            istreambuf_iterator<char>(
                inputFile
            )
        ),
        istreambuf_iterator<char>()
    );


    inputFile.close();


    cout << "Compressed payload loaded."
         << endl;


    cout << "Payload bytes: "
         << compressedBytes.size()
         << endl;


    // ==================================================
    // 7. HUFFMAN DECODE
    // ==================================================

    vector<int> decodedZigzag;


    uint64_t decodedBits = 0;


    HuffmanNode* current =
        root;


    for (
        size_t byteIndex = 0;
        byteIndex < compressedBytes.size();
        byteIndex++
    ) {

        unsigned char byte =
            compressedBytes[byteIndex];


        for (
            int bitIndex = 7;
            bitIndex >= 0;
            bitIndex--
        ) {

            // Stop at validBits
            if (
                decodedBits >=
                header.validBits
            ) {
                break;
            }


            int bit =
                (byte >> bitIndex) & 1;


            if (bit == 0) {

                current =
                    current->left;

            }
            else {

                current =
                    current->right;
            }


            // Leaf node
            if (
                current->left == nullptr &&
                current->right == nullptr
            ) {

                decodedZigzag.push_back(
                    current->value
                );


                current = root;
            }


            decodedBits++;
        }


        if (
            decodedBits >=
            header.validBits
        ) {
            break;
        }
    }


    cout << "Decoded ZigZag values: "
         << decodedZigzag.size()
         << endl;


    // ==================================================
    // 8. EXPECTED PIXELS
    // ==================================================

    uint64_t expectedPixels =
        static_cast<uint64_t>(
            header.width
        )
        *
        static_cast<uint64_t>(
            header.height
        );


    if (
        decodedZigzag.size() !=
        expectedPixels
    ) {

        cout << "ERROR: Decoded pixel count "
             << "does not match expected count."
             << endl;

        deleteTree(root);

        return 1;
    }


    // ==================================================
    // 9. ZIGZAG → RESIDUAL
    // ==================================================

    vector<int> residuals;

    residuals.reserve(
        decodedZigzag.size()
    );


    for (
        int value :
        decodedZigzag
    ) {

        residuals.push_back(
            zigzagDecode(value)
        );
    }


    cout << "Residuals reconstructed."
         << endl;


    // ==================================================
    // 10. RECONSTRUCT PIXELS
    // ==================================================

    vector<unsigned char> reconstructed;

    reconstructed.reserve(
        expectedPixels
    );


    int width =
        static_cast<int>(
            header.width
        );


    for (
        size_t i = 0;
        i < residuals.size();
        i++
    ) {

        int prediction;


        // ------------------------------------------------
        // Average predictor
        // ------------------------------------------------

        if (i == 0) {

            prediction = 0;
        }

        else {

            int left =
                reconstructed[i - 1];


            // First row
            if (i < static_cast<size_t>(width)) {

                prediction =
                    left;
            }

            else {

                int top =
                    reconstructed[
                        i - width
                    ];


                prediction =
                    (
                        left + top
                    ) / 2;
            }
        }


        int pixel =
            prediction +
            residuals[i];


        if (
            pixel < 0 ||
            pixel > 255
        ) {

            cout << "ERROR: Invalid pixel "
                 << "value at index "
                 << i
                 << endl;

            deleteTree(root);

            return 1;
        }


        reconstructed.push_back(
            static_cast<unsigned char>(
                pixel
            )
        );
    }


    cout << "Image pixels reconstructed."
         << endl;


    // ==================================================
    // 11. WRITE RAW IMAGE
    // ==================================================

    ofstream outputFile(
        outputRawPath,
        ios::binary
    );


    if (!outputFile) {

        cout << "ERROR: Could not create "
             << "reconstructed.raw"
             << endl;

        deleteTree(root);

        return 1;
    }


    outputFile.write(
        reinterpret_cast<const char*>(
            reconstructed.data()
        ),
        reconstructed.size()
    );


    outputFile.close();


    cout << "Reconstructed RAW file created."
         << endl;


    // ==================================================
    // 12. VERIFY AGAINST EXPECTED SIZE
    // ==================================================

    cout << endl;

    cout << "========== DECODE RESULTS =========="
         << endl;


    cout << "Expected pixels: "
         << expectedPixels
         << endl;


    cout << "Reconstructed pixels: "
         << reconstructed.size()
         << endl;


    cout << "===================================="
         << endl;


    // ==================================================
    // CLEANUP
    // ==================================================


    // ==================================================
// 13. LOSSLESS VERIFICATION
// ==================================================

ifstream originalFile(
    "../../data/input/image.raw",
    ios::binary
);

if (originalFile) {

    vector<unsigned char> originalPixels(
        (istreambuf_iterator<char>(originalFile)),
        istreambuf_iterator<char>()
    );

    originalFile.close();

    bool lossless =
        (originalPixels == reconstructed);

    cout << endl;
    cout << "========== LOSSLESS VERIFICATION =========="
         << endl;

    if (lossless) {
        cout << "Original pixels: "
             << originalPixels.size()
             << endl;

        cout << "Reconstructed pixels: "
             << reconstructed.size()
             << endl;

        cout << "Pixel-by-pixel comparison: PASSED"
             << endl;

        cout << "LOSSLESS TEST: PASSED"
             << endl;
    }
    else {
        cout << "Pixel-by-pixel comparison: FAILED"
             << endl;

        for (size_t i = 0;
             i < originalPixels.size() &&
             i < reconstructed.size();
             i++) {

            if (originalPixels[i] !=
                reconstructed[i]) {

                cout << "First mismatch at pixel: "
                     << i
                     << endl;

                cout << "Original: "
                     << static_cast<int>(
                            originalPixels[i]
                        )
                     << endl;

                cout << "Reconstructed: "
                     << static_cast<int>(
                            reconstructed[i]
                        )
                     << endl;

                break;
            }
        }

        cout << "LOSSLESS TEST: FAILED"
             << endl;

        deleteTree(root);
        return 1;
    }
}
else {
    cout << endl;
    cout << "========== DECOMPRESSION SUCCESSFUL =========="
         << endl;
    cout << "Note: Original image not present in input folder; skipping pixel-level verification."
         << endl;
}

    deleteTree(root);


    return 0;
}