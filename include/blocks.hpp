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
    // SDL_Color render_flags;
}; // taille à optimiser

struct chunk
{
    bool ready_to_render;

    chunk_coordonate coord;
    block block[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    
    // void read_from_file(Uint32 x, Uint32 y);
};

#endif