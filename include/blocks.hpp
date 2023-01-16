#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <constants.hpp>

struct block
{
    Uint8 id;
};

struct chunk
{
    int compress_value;
    block block[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

#endif