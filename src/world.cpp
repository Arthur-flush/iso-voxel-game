#include <game.hpp>
#include <cstdio>
#include "multithreaded_event_handler.hpp"

World::World(){chunk = NULL;};

typedef chunk CHUNK;

void World::init(uint16_t _x, uint16_t _y, uint16_t _z)
{
    chunk = new CHUNK**[_x];

    for(int x = 0; x < _x ; x++)
    {
        chunk[x] = new CHUNK*[_y];

        for(int y = 0; y < _y ; y++)
        {
            chunk[x][y] = new CHUNK[_z];

            for(int z = 0; z < _z ;z++)
            {
                chunk[x][y][z] = {0};
                chunk[x][y][z].ready_to_render = true;
            }
        }
    }

    max_chunk_coord = {_x-1, _y-1, _z-1};
    min_chunk_coord = {0, 0, 0};

    max_block_coord = {_x*CHUNK_SIZE, _y*CHUNK_SIZE, _z*CHUNK_SIZE};
}

World::~World()
{
    if(!chunk)
        return;
    
    // std::cout << "Deleting chunks" << chunk << "\n";

    for(int x = 0; x <= max_chunk_coord.x; x++)
    {
        for(int y = 0; y <= max_chunk_coord.y; y++)
        {
            delete [] chunk[x][y];
        }

        delete [] chunk[x];
    }

    delete [] chunk;
}

Uint16 World::get_block_id(chunk_coordonate coord, int x, int y, int z)
{
    block *b = get_block(coord, x, y ,z);

    if(!b)
        return BLOCK_EMPTY;
    
    return b->id;
}

Uint16 World::get_block_id_wcoord(int x, int y, int z)
{
    block *b = get_block_wcoord(x, y, z);

    if(!b)
        return BLOCK_EMPTY;
    
    return b->id;
}

block* World::get_block(chunk_coordonate coord, int x, int y, int z)
{
    translate_world_view_position(coord, x, y, z);

    while(x < 0){x += CHUNK_SIZE; coord.x --;}

    while(x >= CHUNK_SIZE){x -= CHUNK_SIZE; coord.x ++;}

    while(y < 0){y += CHUNK_SIZE; coord.y --;}

    while(y >= CHUNK_SIZE){y -= CHUNK_SIZE; coord.y ++;}

    while(z < 0){z += CHUNK_SIZE; coord.z --;}

    while(z >= CHUNK_SIZE){z -= CHUNK_SIZE; coord.z ++;}

    if(coord.x < min_chunk_coord.x || coord.x > max_chunk_coord.x ||
       coord.y < min_chunk_coord.y || coord.y > max_chunk_coord.y ||
       coord.z < min_chunk_coord.z || coord.z > max_chunk_coord.z)
        return NULL;

    if(x < 0 || x >= CHUNK_SIZE || 
       y < 0 || y >= CHUNK_SIZE || 
       z < 0 || z >= CHUNK_SIZE)
        return NULL;
    
    return &chunk[coord.x][coord.y][coord.z].block[x][y][z];
}

block* World::get_block_wcoord(int x, int y, int z)
{
    translate_world_view_wposition(x, y, z);

    chunk_coordonate coord = {x / CHUNK_SIZE,
                             y / CHUNK_SIZE,
                             z / CHUNK_SIZE};
    
    x %= CHUNK_SIZE;
    y %= CHUNK_SIZE;
    z %= CHUNK_SIZE;

    if(coord.x < min_chunk_coord.x || coord.x > max_chunk_coord.x ||
       coord.y < min_chunk_coord.y || coord.y > max_chunk_coord.y ||
       coord.z < min_chunk_coord.z || coord.z > max_chunk_coord.z)
        return NULL;

    return &chunk[coord.x][coord.y][coord.z].block[x][y][z];
}

Uint16 World::get_opaque_block_id(chunk_coordonate coord, int x, int y, int z)
{
    block *b = get_block(coord, x, y ,z);

    if(!b || b->id >= BLOCK_TRANSPARENT_LIMIT)
        return BLOCK_EMPTY;
    
    return b->id;
}

block_coordonate World::convert_wcoord(int x, int y, int z)
{
    block_coordonate bc;

    bc.x = x%CHUNK_SIZE;
    bc.y = y%CHUNK_SIZE;
    bc.z = z%CHUNK_SIZE;

    bc.chunk.x = x/CHUNK_SIZE;
    bc.chunk.y = y/CHUNK_SIZE;
    bc.chunk.z = z/CHUNK_SIZE;

    return bc;
}

world_coordonate World::convert_coord(block_coordonate bc)
{
    world_coordonate coord;

    coord.x = bc.x + bc.chunk.x*CHUNK_SIZE;
    coord.y = bc.y + bc.chunk.y*CHUNK_SIZE;
    coord.z = bc.z + bc.chunk.z*CHUNK_SIZE;

    if(coord.x > max_block_coord.x)
    {
        std::cout << "\n" << bc.chunk.x << " " << bc.x << " " << max_chunk_coord.x;
    }

    return coord;
}

void World::translate_world_view_position(chunk_coordonate& coord, int& x, int& y, int& z)
{
    if(world_view_position == 1)
    {
        int ytmp = y;
        int cytmp = coord.y;

        y = x;
        x = ytmp;

        coord.y = coord.x;
        coord.x = cytmp;

        ytmp = max_block_coord.y - y - coord.y*CHUNK_SIZE;
        y = ytmp%CHUNK_SIZE;
        coord.y = ytmp/CHUNK_SIZE;
    }
    else if(world_view_position == 3)
    {
        int ytmp = y;
        int cytmp = coord.y;

        y = x;
        x = ytmp;

        coord.y = coord.x;
        coord.x = cytmp;

        ytmp = max_block_coord.x - x - coord.x*CHUNK_SIZE;
        x = ytmp%CHUNK_SIZE;
        coord.x = ytmp/CHUNK_SIZE;
    }
    else if(world_view_position == 2)
    {
        y = max_block_coord.y - y - coord.y*CHUNK_SIZE;
        x = max_block_coord.x - x - coord.x*CHUNK_SIZE;

        coord.y = y/CHUNK_SIZE;
        coord.x = x/CHUNK_SIZE;
        
        y = y%CHUNK_SIZE;
        x = x%CHUNK_SIZE;
    }
}

void World::translate_world_view_wposition(int& x, int& y, int& z)
{
    if(world_view_position == 1)
    {
        int ytmp = y;

        y = x;
        x = ytmp;

        y = max_block_coord.y-y;
    }
    else if(world_view_position == 3)
    {
        int ytmp = y;

        y = x;
        x = ytmp;

        x = max_block_coord.x-x;
    }
    else if(world_view_position == 2)
    {
        y = max_block_coord.y-y;
        x = max_block_coord.x-x;
    }
}

void World::compress_chunk(int x, int y, int z)
{
    // struct chunk* c = &chunk[x][y][z];

    // c->compress_value = CHUNK_NON_UNIFORM;

    // int id = c->block[0][0][0].id;

    // for(int i = 1; i < CHUNK_SIZE; i++)
    //     if(c->block[0][0][i].id != id)
    //         return;

    // Uint64 *c2 = (Uint64*)(&c->block);

    // int size = (CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE)/8;

    // for(int i = 0; i < size; i++)
    //     if(c2[0] != c2[i])
    //         return;
    
    // c->compress_value = id;

    struct chunk* c = &chunk[x][y][z];
    c->compress_value = CHUNK_NON_UNIFORM;
    Uint8 val = c->block[0][0][0].id;

    for(int i = 0; i < CHUNK_SIZE; i++)
        for(int j = 0; j < CHUNK_SIZE; j++)
            for(int k = 0; k < CHUNK_SIZE; k++)
                if(c->block[i][j][k].id != val)
                    return;
    
    c->compress_value = val;

    // std::cout << "\nCHUNK COMPRESSED AT COORD " << x << " " << y << " " << z << " ";
    // std::cout << "\tWITH ID " << id;
}

void World::compress_all_chunks()
{
    for(int x = 0; x <= max_chunk_coord.x; x++)
    for(int y = 0; y <= max_chunk_coord.y; y++)
    for(int z = 0; z <= max_chunk_coord.z; z++)
    {
        compress_chunk(x, y, z);
    }
}

Uint32 World::line_presenceF(world_coordonate start, world_coordonate jump)
{
    // No valid coord checking
    // Positions arent output
    // Both opaque and transparent blocks are checked, but the function ends when an opaque is found
    // Jumps are positive

    if(start.x > max_block_coord.x || start.y > max_block_coord.y || start.x > max_block_coord.z)
        return 0;

    struct chunk* c;
    block_coordonate bc = convert_wcoord(start.x, start.y, start.z);
    Uint16 oid = BLOCK_EMPTY;
    Uint16 tid = BLOCK_EMPTY;
    bool chunk_end = false;

    while(bc.chunk.x <= max_chunk_coord.x && bc.chunk.y <= max_chunk_coord.y && bc.chunk.z <= max_chunk_coord.z)
    {
        c = &chunk[bc.chunk.x][bc.chunk.y][bc.chunk.z];

        // std::cout << "\nboucle 1" << bc.chunk.x << " " << max_chunk_coord.x << " " << c->compress_value;

        if(c->compress_value == CHUNK_NON_UNIFORM)
        {
            chunk_end = false;

            while(!chunk_end)
            {
                oid = c->block[bc.x][bc.y][bc.z].id;

                if(oid >= BLOCK_TRANSPARENT_LIMIT)
                {
                    tid = oid;
                }
                else if(oid != BLOCK_EMPTY)
                {
                    // std::cout << "\n\t yo1 " << c->compress_value;
                    return oid + (tid<<8);
                }

                bc.x += jump.x;
                bc.y += jump.y;
                bc.z += jump.z;

                if(bc.x == CHUNK_SIZE)
                {
                    bc.chunk.x += jump.x;
                    bc.x = 0;
                    chunk_end = true;
                }
                if(bc.y == CHUNK_SIZE)
                {
                    bc.chunk.y += jump.y;
                    bc.y = 0;
                    chunk_end = true;
                }
                if(bc.z == CHUNK_SIZE)
                {
                    bc.chunk.z += jump.z;
                    bc.z = 0;
                    chunk_end = true;
                }
            }
        }
        else if(c->compress_value != BLOCK_EMPTY && c->compress_value < BLOCK_TRANSPARENT_LIMIT)
        {
            return c->compress_value + (tid<<8);
        }
        else
        {
            if(c->compress_value != BLOCK_EMPTY)
                tid = c->compress_value;
            
            chunk_end = false;

            while(!chunk_end)
            {
                bc.x += jump.x;
                bc.y += jump.y;
                bc.z += jump.z;

                if(bc.x == CHUNK_SIZE)
                {
                    bc.chunk.x += jump.x;
                    bc.x = 0;
                    chunk_end = true;
                }
                if(bc.y == CHUNK_SIZE)
                {
                    bc.chunk.y += jump.y;
                    bc.y = 0;
                    chunk_end = true;
                }
                if(bc.z == CHUNK_SIZE)
                {
                    bc.chunk.z += jump.z;
                    bc.z = 0;
                    chunk_end = true;
                }
            }

        }

    }

    return 0;
}

// line_presence World::line_visiblePR(world_coordonate start, world_coordonate jump)
// {
//     // Valid coord checking
//     // Positions are output
//     // Both opaque and transparent blocks are checked, but the function ends when an opaque is found
//     // Jumps can be negative

//     line_presence lp = {0};

//     if(start.x > max_block_coord.x || start.y > max_block_coord.y || start.z > max_block_coord.z)
//     {
//         std::cout << "\nline_visiblePR : Wrong position given at " << start.x << ' ' << start.y << ' ' << start.z;
//         return lp;
//     }

//     struct chunk* c;
//     bool chunk_end = false;

//     int diff = max_block_coord.z - start.z;
//     start.x += diff;
//     start.y += diff;
//     start.z += diff;

//     int id;
//     block_coordonate bc = convert_wcoord(start.x, start.y, start.z);

//     while
//     (
//           bc.chunk.x >= 0 && bc.chunk.x <= max_chunk_coord.x &&
//           bc.chunk.y >= 0 && bc.chunk.y <= max_chunk_coord.y &&
//           bc.chunk.x >= 0
//     ){
//         c = &chunk[bc.chunk.x][bc.chunk.y][bc.chunk.z];

//         if(c->compress_value == CHUNK_NON_UNIFORM)
//         {
//             chunk_end = false;

                
//             while(!chunk_end)
//             {
//                 id = c->block[bc.x][bc.y][bc.z].id;

//                 if(id >= BLOCK_TRANSPARENT_LIMIT && !lp.tid)
//                 {
//                     lp.tid = id;
//                     lp.twcoord = convert_coord(bc);
//                 }
//                 else if(id != BLOCK_EMPTY)
//                 {
//                     // std::cout << "yo\n";
//                     lp.oid = id;
//                     lp.owcoord = convert_coord(bc);
//                     return lp;
//                 }

//                 bc.x += jump.x;
//                 bc.y += jump.y;
//                 bc.z += jump.z;

//                 if(bc.x == CHUNK_SIZE || bc.x == -1)
//                 {
//                     bc.chunk.x += jump.x;
//                     bc.x = jump.x < 0 ? CHUNK_SIZE-1 : 0;
//                     chunk_end = true;
//                 }
//                 if(bc.y == CHUNK_SIZE || bc.y == -1)
//                 {
//                     bc.chunk.y += jump.y;
//                     bc.y = jump.y < 0 ? CHUNK_SIZE-1 : 0;
//                     chunk_end = true;
//                 }
//                 if(bc.z == -1)
//                 {
//                     bc.chunk.z += jump.z;
//                     bc.z = CHUNK_SIZE-1;
//                     chunk_end = true;
//                 }
//             }
//         }

//         else if(c->compress_value != BLOCK_EMPTY && c->compress_value < BLOCK_TRANSPARENT_LIMIT)
//         {
//             // std::cout << "yo2\n";
//             lp.oid = c->compress_value;
//             lp.owcoord = convert_coord(bc);
//             return lp;
//         }

//         else
//         {
//             if(c->compress_value != BLOCK_EMPTY && !lp.tid)
//             {
//                 lp.tid = c->compress_value;
//                 lp.twcoord = convert_coord(bc);
//             }
            
//             chunk_end = false;

//             while(!chunk_end)
//             {
//                 bc.x += jump.x;
//                 bc.y += jump.y;
//                 bc.z += jump.z;

//                 if(bc.x == CHUNK_SIZE || bc.x == -1)
//                 {
//                     bc.chunk.x += jump.x;
//                     bc.x = jump.x < 0 ? CHUNK_SIZE-1 : 0;
//                     chunk_end = true;
//                 }
//                 if(bc.y == CHUNK_SIZE || bc.y == -1)
//                 {
//                     bc.chunk.y += jump.y;
//                     bc.y = jump.y < 0 ? CHUNK_SIZE-1 : 0;
//                     chunk_end = true;
//                 }
//                 if(bc.z == -1)
//                 {
//                     bc.chunk.z += jump.z;
//                     bc.z = CHUNK_SIZE-1;
//                     chunk_end = true;
//                 }
//             }

//         }
//     }

//     return lp;
// }

line_presence World::line_visiblePR(world_coordonate start, world_coordonate jump)
{
    line_presence lp;
    lp.oid = 0;
    lp.tid = 0;
    lp.owcoord = {0, 0, 0};
    lp.twcoord = {0, 0, 0};

    if(start.x > max_block_coord.x || start.y > max_block_coord.y || start.z > max_block_coord.z)
    {
        std::cout << "\nline_visiblePR : Wrong position given at " << start.x << ' ' << start.y << ' ' << start.z;
        return lp;
    }

    int id;
    block_coordonate bc;
    world_coordonate wc;

    while
    (
        start.x >= 0 && start.y >= 0 && start.z >= 0
    ){
        wc.x = start.x;
        wc.y = start.y;
        wc.z = start.z;

        id = get_block_id_wcoord(wc.x, wc.y, wc.z);

        if(id)
        {
            if(id < BLOCK_TRANSPARENT_LIMIT)
            {
                lp.oid = id;
                lp.owcoord.x = wc.x;
                lp.owcoord.y = wc.y;
                lp.owcoord.z = wc.z;
                return lp;
            }
            else if(!lp.tid)
            {
                lp.tid = id;
                lp.twcoord.z = wc.z;
            }
        }


        start.x --;
        start.y --;
        start.z --;
    }
    
    return lp;
}



int World::save_to_file(std::string filename) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        std::cout << "Error opening file " << filename << "\n";
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    chunk_coordonate tmp;
    tmp.x = max_chunk_coord.x + 1;
    tmp.y = max_chunk_coord.y + 1;
    tmp.z = max_chunk_coord.z + 1;

    fwrite(&(tmp.x), sizeof(chunk_coordonate::x), 1, file);
    fwrite(&(tmp.y), sizeof(chunk_coordonate::y), 1, file);
    fwrite(&(tmp.z), sizeof(chunk_coordonate::z), 1, file);

    // std::cout << "max_chunk_coord.x = " << tmp.x << std::endl;
    // std::cout << "max_chunk_coord.y = " << tmp.y << std::endl;
    // std::cout << "max_chunk_coord.z = " << tmp.z << std::endl;


    for (int x = 0; x < tmp.x; x++) {
        for (int y = 0; y < tmp.y; y++) {
            for (int z = 0; z < tmp.z; z++) {
                switch (chunk[x][y][z].compress_value) {
                    case CHUNK_EMPTY: {// chunk is empty so we write a 0
                        Uint8 empty = 0;
                        fwrite(&(empty), sizeof(empty), 1, file);
                        break;
                    }

                    case CHUNK_NON_UNIFORM: {// chunk is not uniform so we write a 2 and we use "run length encoding" (RLE) to write the blocks ids
                        // RLE
                        Uint8 non_uniform = 2;
                        fwrite(&(non_uniform), sizeof(non_uniform), 1, file);
                        Uint8 current_id = chunk[x][y][z].block[0][0][0].id;
                        Uint16 current_count = 0;
                        for (int i = 0; i < CHUNK_SIZE; i++) {
                            for (int j = 0; j < CHUNK_SIZE; j++) {
                                for (int k = 0; k < CHUNK_SIZE; k++) {
                                    if (chunk[x][y][z].block[i][j][k].id == current_id) {
                                        current_count++;
                                    }
                                    else {
                                        fwrite(&(current_count), sizeof(current_count), 1, file);
                                        fwrite(&(current_id), sizeof(current_id), 1, file);
                                        // std::cout << "current_count = " << current_count << std::endl;
                                        // std::cout << "current_id = " << (int)current_id << std::endl;
                                        current_id = chunk[x][y][z].block[i][j][k].id;
                                        current_count = 1;
                                    }
                                }
                            }
                        }
                        fwrite(&(current_count), sizeof(current_count), 1, file);
                        fwrite(&(current_id), sizeof(current_id), 1, file);
                        // std::cout << "current_count = " << current_count << std::endl;
                        // std::cout << "current_id = " << (int)current_id << std::endl;
                        break;
                    }

                    default: {// chunk is full of the same block so we write a 1 and the block id
                        Uint8 full = 1;
                        fwrite(&(full), sizeof(full), 1, file);
                        fwrite(&(chunk[x][y][z].block[0][0][0].id), sizeof(chunk[x][y][z].block[0][0][0].id), 1, file);
                        break;   
                    }
                }
            }
        }
    }

    fclose(file);
    return SAVE_ERROR_NONE;

}

void fill_chunk(chunk* chunk, Uint8 id) {
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                chunk->block[i][j][k].id = id;
            }
        }
    }
}

int World::load_from_file(std::string filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cout << "Error opening file " << filename << "\n";
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    // if (chunk) {
    //     for (int x = 0; x < max_chunk_coord.x; x++) {
    //         for (int y = 0; y < max_chunk_coord.y; y++) {
    //             delete[] chunk[x][y];
    //         }
    //         delete[] chunk[x];
    //     }
    //     delete[] chunk;
    // }



    chunk_coordonate tmp;

    fread(&(tmp.x), sizeof(chunk_coordonate::x), 1, file);
    fread(&(tmp.y), sizeof(chunk_coordonate::y), 1, file);
    fread(&(tmp.z), sizeof(chunk_coordonate::z), 1, file);

    init(tmp.x, tmp.y, tmp.z);

    for (int x = 0; x < tmp.x; x++) {
        for (int y = 0; y < tmp.y; y++) {
            for (int z = 0; z < tmp.z; z++) {
                Uint8 compress_value;
                fread(&(compress_value), sizeof(compress_value), 1, file);
                switch (compress_value) {
                    case 0: // chunk is empty
                        chunk[x][y][z].compress_value = CHUNK_EMPTY;
                        fill_chunk(&(chunk[x][y][z]), 0);
                        break;
                    
                    case 1: // chunk is full of the same block
                        Uint8 id;
                        fread(&(id), sizeof(id), 1, file);
                        fill_chunk(&(chunk[x][y][z]), id);
                        chunk[x][y][z].compress_value = id;
                        break;   
                    
                    case 2: // chunk is not uniform
                        // RLE
                        chunk[x][y][z].compress_value = CHUNK_NON_UNIFORM;
                        Uint8 current_id;
                        Uint16 current_count;
                        for (int i = 0; i < CHUNK_SIZE; i++) {
                            for (int j = 0; j < CHUNK_SIZE; j++) {
                                for (int k = 0; k < CHUNK_SIZE; k++) {
                                    if (current_count == 0) {
                                        fread(&current_count, sizeof(current_count), 1, file);
                                        fread(&current_id, sizeof(current_id), 1, file);
                                    }
                                    chunk[x][y][z].block[i][j][k].id = current_id;
                                    current_count--;
                                }
                            }
                        }
                        break;
                
                }
            }
        }
    }
    
    fclose(file);
    return SAVE_ERROR_NONE;
}