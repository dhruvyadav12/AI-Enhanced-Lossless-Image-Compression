#ifndef HUFFMAN_FORMAT_H
#define HUFFMAN_FORMAT_H

#include <cstdint>


struct HuffmanEntry {

    int32_t symbol;

    uint32_t frequency;
};


#endif