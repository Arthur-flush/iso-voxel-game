#ifndef WORLD_HPP
#define WORLD_HPP

#include <blocks.hpp>
#include <vector>

struct World
{
    World();
    // World(uint16_t, uint16_t, uint16_t);
    ~World();

    void init(uint16_t, uint16_t, uint16_t);

    chunk_coordonate max_chunk_coord;
    chunk_coordonate min_chunk_coord;

    chunk_coordonate max_block_coord;

    // std::vector<std::vector<std::vector<chunk>>> chunk;

    chunk ***chunk;

    block* get_block(chunk_coordonate, int, int, int);
    block* get_block_wcoord(int, int, int);
    Uint16 get_block_id(chunk_coordonate, int, int, int);
    Uint16 get_block_id_wcoord(int, int, int);

    Uint16 get_opaque_block_id(chunk_coordonate, int, int, int);
    
    block_coordonate convert_wcoord(int, int, int);
    // bool is_block_highlighted();
};

#endif