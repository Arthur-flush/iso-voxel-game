#ifndef WORLD_HPP
#define WORLD_HPP

#include <blocks.hpp>
#include <vector>

struct line_presence
{
    Uint16 oid;
    Uint16 tid;

    world_coordonate owcoord;
    world_coordonate twcoord;
};

struct World
{
    World();
    // World(uint16_t, uint16_t, uint16_t);
    ~World();

    void init(uint16_t, uint16_t, uint16_t);

    chunk_coordonate max_chunk_coord;
    chunk_coordonate min_chunk_coord;

    chunk_coordonate max_block_coord;

    chunk ***chunk;

    block* get_block(chunk_coordonate, int, int, int);
    block* get_block_wcoord(int, int, int);
    Uint16 get_block_id(chunk_coordonate, int, int, int);
    Uint16 get_block_id_wcoord(int, int, int);

    Uint16 get_opaque_block_id(chunk_coordonate, int, int, int);
    
    block_coordonate convert_wcoord(int, int, int);
    world_coordonate convert_coord(block_coordonate);

    /* load and save functions
     * returns an int that encode an error code
     * 0 being no errors
     * 
     */
    int save_to_file(std::string filename);
    int load_from_file(std::string filename);

    void translate_world_view_position(chunk_coordonate&, int&, int&, int&);
    void translate_world_view_wposition(int&, int&, int&);
    // void translate_world_view_jposition(int&, int&, int&);
    int world_view_position;

    void compress_chunk(int, int, int);
    void compress_all_chunks();

    /**** GET PRESENCE ON A PREDEFINED LINE OF THE WORLD ****/

    // F := fast, but risky because no coord checking
    // P := get position too
    // O := get only opaque blocks
    // T := get only transparent blocks
    // R := reverse, if the jumps are negative
    // nox/y/z := don't check x/y/z

    Uint32 line_presenceF(world_coordonate, world_coordonate);

    line_presence line_visiblePR(world_coordonate, world_coordonate);
    /********************************************************/
};

#endif