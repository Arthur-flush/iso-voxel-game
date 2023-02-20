#ifndef WORLD_HPP
#define WORLD_HPP

#include <mutex>

#include <blocks.hpp>
#include <coords.hpp>

struct World
{
    World();
    ~World();

    void init(uint16_t, uint16_t, uint16_t);
    void free_chunks();

    chunk_coordonate max_chunk_coord;
    chunk_coordonate min_chunk_coord;

    chunk_coordonate max_block_coord;

    chunk ***chunks;

    block* get_block(chunk_coordonate, int, int, int);
    block* get_block_wcoord(int, int, int);
    Uint16 get_block_id(chunk_coordonate, int, int, int);
    Uint16 get_block_id_wcoord(int, int, int);

    Uint16 get_opaque_block_id(chunk_coordonate, int, int, int);
    
    block_coordonate convert_wcoord(int, int, int);
    world_coordonate convert_coord(block_coordonate);

    void translate_world_view_position(chunk_coordonate&, int&, int&, int&);
    void translate_world_view_wposition(int&, int&, int&);
    void translate_world_view_wpositionf(float&, float&);
    void invert_wvp(int &x, int &y);
    int world_view_position;

    void compress_chunk(int, int, int);
    void compress_all_chunks();

    // get the presence of blocks in the path from the given coord to the sun/moon
    // the opaque id is given from the first bit
    // the transparent id is given from the last bit
    // like everything in the world struct, it automaticly take car of world view position transformation
    Uint32 shadow_caster_presence(world_coordonate);

    // Please consider using STHREAD_OP_SINGLE_CHUNK_POS or STHREAD_OP_SINGLE_BLOCK_VISBLE instead
    // This one does NOT take care of projection grid refresh
    // Modify the block at the given world coordonate
    // Like everything in the world struct, it automaticly take car of world view position transformation
    bool modify_block(world_coordonate, int);

    int highest_nonemptychunk;
    int find_highest_nonemptychunk();

    /* load and save functions
     * returns an int that encode an error code
     * 0 being no errors
     * 
     */
    int save_to_file(const std::string& filename);
    int load_from_file(const char* filename);
};

#endif