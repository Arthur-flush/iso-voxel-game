#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <SDL2/SDL.h>
#include <constants.hpp>
#include <coords.hpp>

struct block
{
    Uint8 id;
};

struct chunk
{
    bool ready_to_render; // old je pense
    chunk_coordonate coord; // same

    int compress_value;
    block block[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

#endif