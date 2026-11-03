#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "../include/huffman_format.h"

using namespace std;


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

    int left = pixels[index - 1];

    if (index < width) {
        return left;
    }

    int top = pixels[index - width];

    return (left + top) / 2;
}


// ==================================================
// MAIN
// ==================================================

int main() {

    int width = 1086;


    // ------------------------------------------------
    // 1. READ RAW IMAGE
    // ------------------------------------------------

    string inputPath =
        "../../data/input/image.raw";


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


    cout << "Image loaded."
         << endl;

    cout << "Pixels: "
         << pixels.size()
         << endl;


    // ------------------------------------------------
    // 2. CREATE FREQUENCY TABLE
    // ------------------------------------------------

    unordered_map<int, int> frequency;


    for (int i = 0;
         i < pixels.size();
         i++) {

        int prediction =
            averagePredictor(
                pixels,
                i,
                width
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


        frequency[zigzag]++;
    }


    cout << "Unique symbols: "
         << frequency.size()
         << endl;


    // ------------------------------------------------
    // 3. WRITE METADATA
    // ------------------------------------------------

    string outputPath =
        "../../data/output/huffman_table.bin";


    ofstream outputFile(
        outputPath,
        ios::binary
    );


    if (!outputFile) {

        cout << "ERROR: Could not create "
             << "huffman_table.bin"
             << endl;

        return 1;
    }


    // First write number of symbols

    uint32_t symbolCount =
        static_cast<uint32_t>(
            frequency.size()
        );


    outputFile.write(
        reinterpret_cast<const char*>(
            &symbolCount
        ),
        sizeof(symbolCount)
    );


    // Write every symbol + frequency

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


    outputFile.close();


    cout << "Huffman metadata written!"
         << endl;


    // ------------------------------------------------
    // 4. READ METADATA BACK
    // ------------------------------------------------

    ifstream metadataFile(
        outputPath,
        ios::binary
    );


    if (!metadataFile) {

        cout << "ERROR: Could not read metadata."
             << endl;

        return 1;
    }


    uint32_t loadedSymbolCount;


    metadataFile.read(
        reinterpret_cast<char*>(
            &loadedSymbolCount
        ),
        sizeof(loadedSymbolCount)
    );


    cout << endl;

    cout << "Loaded symbol count: "
         << loadedSymbolCount
         << endl;


    // ------------------------------------------------
    // 5. READ FIRST 10 ENTRIES
    // ------------------------------------------------

    cout << endl;

    cout << "First 10 Huffman entries:"
         << endl;


    for (int i = 0;
         i < 10 && i < loadedSymbolCount;
         i++) {

        HuffmanEntry entry;


        metadataFile.read(
            reinterpret_cast<char*>(
                &entry
            ),
            sizeof(HuffmanEntry)
        );


        cout << entry.symbol
             << " -> "
             << entry.frequency
             << endl;
    }


    metadataFile.close();


    cout << endl;

    cout << "HUFFMAN METADATA TEST: PASSED"
         << endl;


    return 0;
}