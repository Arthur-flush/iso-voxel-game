#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <SDL2/SDL.h>
#include <constants.hpp>

struct chunk_coordonate
{
    int x;
    int y;
    int z;
};

struct world_coordonate
{
    int x;
    int y;
    int z;
};

struct block_coordonate
{
    chunk_coordonate chunk;
    int x, y, z;
};

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