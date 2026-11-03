#ifndef AIC_FORMAT_H
#define AIC_FORMAT_H

#include <cstdint>


// ==================================================
// AIC FILE HEADER
// ==================================================

struct AICHeader {

    // Magic number
    // Identifies this as an AIC file

    char magic[4];


    // Format version

    uint8_t version;


    // Image dimensions

    uint32_t width;

    uint32_t height;


    // Number of channels
    //
    // 1 = grayscale
    // 3 = RGB

    uint8_t channels;


    // Predictor type
    //
    // 0 = Left
    // 1 = Average
    // 2 = Paeth
    // 3 = AI

    uint8_t predictor;


    // Number of valid bits
    // in compressed bitstream

    uint64_t validBits;


    // Number of unique Huffman symbols

    uint32_t symbolCount;
};


#endif