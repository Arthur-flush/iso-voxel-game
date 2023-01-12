#include <game.hpp>

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
    while(x < 0){x += CHUNK_SIZE; coord.x --;}

    while(x >= CHUNK_SIZE){x -= CHUNK_SIZE; coord.x ++;}

    while(y < 0){y += CHUNK_SIZE; coord.y --;}

    while(y >= CHUNK_SIZE){y -= CHUNK_SIZE; coord.y ++;}

    while(z < 0){z += CHUNK_SIZE; coord.z --;}

    while(z >= CHUNK_SIZE){z -= CHUNK_SIZE; coord.z ++;}

    // if(x < 0)
    // {
    //     coord.x -= (-x/CHUNK_SIZE)+1;
    //     x = (-x%CHUNK_SIZE);
    // }
    // else if(x >= CHUNK_SIZE)
    // {
    //     coord.x += x/CHUNK_SIZE;
    //     x = x%CHUNK_SIZE;
    // }

    // if(y < 0)
    // {
    //     coord.y -= (-y/CHUNK_SIZE)+1;
    //     y = (-y%CHUNK_SIZE);
    // }
    // else if(y >= CHUNK_SIZE)
    // {
    //     coord.y += y/CHUNK_SIZE;
    //     y = y%CHUNK_SIZE;
    // }

    // if(z < 0)
    // {
    //     coord.z -= (-z/CHUNK_SIZE)+1;
    //     z = (-z%CHUNK_SIZE);
    // }
    // else if(z >= CHUNK_SIZE)
    // {
    //     coord.z += z/CHUNK_SIZE;
    //     z = z%CHUNK_SIZE;
    // }

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
