#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <constants.hpp>
#include <SDL2/SDL.h>

struct block
{
    Uint8 id;
};

struct chunk
{
    int compress_value;
    block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

#endif