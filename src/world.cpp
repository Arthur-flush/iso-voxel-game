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
    // Exchange x and y, then reverse y
    if(world_view_position == 1)
    {
        int ytmp = y;
        int cytmp = coord.y;

        y = x;
        x = ytmp;

        coord.y = coord.x;
        coord.x = cytmp;

        coord.y = max_chunk_coord.y-coord.y;
        y = CHUNK_SIZE-1-y;
    }
    // Exchange x and y, then reverse x
    else if(world_view_position == 3)
    {
        int ytmp = y;
        int cytmp = coord.y;

        y = x;
        x = ytmp;

        coord.y = coord.x;
        coord.x = cytmp;

        coord.x = max_chunk_coord.x-coord.x;
        x = CHUNK_SIZE-1-x;
    }
    // Reverse both x and y
    else if(world_view_position == 2)
    {
        coord.y = max_chunk_coord.y-coord.y;
        y = CHUNK_SIZE-1-y;
    
        coord.x = max_chunk_coord.x-coord.x;
        x = CHUNK_SIZE-1-x;
    }
}

void World::translate_world_view_wposition(int& x, int& y, int& z)
{
    if(world_view_position == 1)
    {
        int ytmp = y;

        y = x;
        x = ytmp;

        y = max_block_coord.y-1-y;
    }
    else if(world_view_position == 3)
    {
        int ytmp = y;

        y = x;
        x = ytmp;

        x = max_block_coord.x-1-x;
    }
    else if(world_view_position == 2)
    {
        y = max_block_coord.y-1-y;
        x = max_block_coord.x-1-x;
    }
}

void World::translate_world_view_wpositionf(float& x, float& y)
{
    if(world_view_position == 1)
    {
        float ytmp = y;

        y = x;
        x = ytmp;

        y = max_block_coord.y-1.0-y;
    }
    else if(world_view_position == 3)
    {
        float ytmp = y;

        y = x;
        x = ytmp;

        x = max_block_coord.x-1.0-x;
    }
    else if(world_view_position == 2)
    {
        y = max_block_coord.y-1.0-y;
        x = max_block_coord.x-1.0-x;
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

Uint32 World::shadow_caster_presence(world_coordonate start)
{
    // No valid coord checking
    // Positions arent output
    // Both opaque and transparent blocks are checked, but the function ends when an opaque is found
    // Jumps are positive

    if(start.x > max_block_coord.x || start.y > max_block_coord.y || start.z > max_block_coord.z)
        return 0;

    struct chunk* c;
    block_coordonate bc = convert_wcoord(start.x, start.y, start.z);
    Uint16 oid = BLOCK_EMPTY;
    Uint16 tid = BLOCK_EMPTY;
    bool chunk_end = false;

    if(bc.chunk.y > max_chunk_coord.y)
        return 0;

    while(bc.chunk.x <= max_chunk_coord.x && bc.chunk.z <= max_chunk_coord.z)
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

                bc.x += 1;
                bc.z += 1;

                if(bc.x == CHUNK_SIZE)
                {
                    bc.chunk.x ++;
                    bc.x = 0;
                    chunk_end = true;
                }
                if(bc.z == CHUNK_SIZE)
                {
                    bc.chunk.z ++;
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
            
            int max = bc.x > bc.z ? bc.x : bc.z;
            max = CHUNK_SIZE-max;

            bc.x += max;
            bc.z += max;

            if(bc.x == CHUNK_SIZE)
            {
                bc.chunk.x ++;
                bc.x = 0;
            }
            if(bc.z == CHUNK_SIZE)
            {
                bc.chunk.z ++;
                bc.z = 0;
            }

        }

    }

    return 0;
}

bool World::modify_block(world_coordonate wcoord, int id)
{
    translate_world_view_wposition(wcoord.x, wcoord.y, wcoord.z);

    block_coordonate coord = convert_wcoord(wcoord.x, wcoord.y, wcoord.z);

    if(coord.x < min_chunk_coord.x || coord.x > max_chunk_coord.x ||
       coord.y < min_chunk_coord.y || coord.y > max_chunk_coord.y ||
       coord.z < min_chunk_coord.z || coord.z > max_chunk_coord.z)
        return false;
    
    chunk[coord.chunk.x][coord.chunk.y][coord.chunk.z].block[coord.x][coord.y][coord.z].id = id;

    compress_chunk(coord.chunk.x, coord.chunk.y, coord.chunk.z);

    return true;
}