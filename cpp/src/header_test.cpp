#include <iostream>
#include <fstream>
#include "../include/aic_format.h"

using namespace std;

int main() {

    AICHeader header;

    // Magic number
    header.magic[0] = 'A';
    header.magic[1] = 'I';
    header.magic[2] = 'C';
    header.magic[3] = '1';

    // Version
    header.version = 1;

    // Image information
    header.width = 1086;
    header.height = 1448;
    header.channels = 1;

    // Average Predictor
    header.predictor = 1;

    // Current Average Predictor result
    header.validBits = 6338034;

    // Number of Huffman symbols
    header.symbolCount = 232;


    // ------------------------------------------
    // WRITE HEADER
    // ------------------------------------------

    ofstream output(
        "../../data/output/header_test.aic",
        ios::binary
    );

    if (!output) {

        cout << "ERROR: Could not create file."
             << endl;

        return 1;
    }

    output.write(
        reinterpret_cast<const char*>(&header),
        sizeof(AICHeader)
    );

    output.close();


    cout << "AIC header written successfully!"
         << endl;

    cout << "Header size: "
         << sizeof(AICHeader)
         << " bytes"
         << endl;


    // ------------------------------------------
    // READ HEADER BACK
    // ------------------------------------------

    AICHeader loadedHeader;

    ifstream input(
        "../../data/output/header_test.aic",
        ios::binary
    );

    if (!input) {

        cout << "ERROR: Could not read file."
             << endl;

        return 1;
    }

    input.read(
        reinterpret_cast<char*>(&loadedHeader),
        sizeof(AICHeader)
    );

    input.close();


    // ------------------------------------------
    // DISPLAY
    // ------------------------------------------

    cout << endl;

    cout << "========== HEADER =========="
         << endl;

    cout << "Magic: "
         << loadedHeader.magic[0]
         << loadedHeader.magic[1]
         << loadedHeader.magic[2]
         << loadedHeader.magic[3]
         << endl;

    cout << "Version: "
         << static_cast<int>(
                loadedHeader.version
            )
         << endl;

    cout << "Width: "
         << loadedHeader.width
         << endl;

    cout << "Height: "
         << loadedHeader.height
         << endl;

    cout << "Channels: "
         << static_cast<int>(
                loadedHeader.channels
            )
         << endl;

    cout << "Predictor: "
         << static_cast<int>(
                loadedHeader.predictor
            )
         << endl;

    cout << "Valid bits: "
         << loadedHeader.validBits
         << endl;

    cout << "Huffman symbols: "
         << loadedHeader.symbolCount
         << endl;

    cout << "============================"
         << endl;


    return 0;
}