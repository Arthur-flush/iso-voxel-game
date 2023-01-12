#include <game.hpp>

Uint64 Get_time_ms();

void Render_Engine::refresh_line_visible2(int x, int y, int z, world_coordonate& jump)
{
    block b;
    screen_block *sb = projection_grid.get_pos_world(x, y, z);

    sb->block.id = BLOCK_EMPTY;
    sb->transparent_block.id = BLOCK_EMPTY;
    sb->height = 0;
    sb->height_transparent = 0;

    block_coordonate bc = world.convert_wcoord(x, y, z);

    while(bc.chunk.x >= 0 && bc.chunk.y >= 0 && bc.chunk.z >= 0)
    {
        // b.id = world.get_block_id(bc.chunk, bc.x, bc.y, bc.z);

        b.id = world.chunk[bc.chunk.x][bc.chunk.y][bc.chunk.z].block[bc.x][bc.y][bc.z].id;

        if(b.id)
        {
            if(b.id < BLOCK_TRANSPARENT_LIMIT)
            {
                sb->block.id = b.id;

                sb->height = bc.z+bc.chunk.z*CHUNK_SIZE;
                sb->x = bc.x+bc.chunk.x*CHUNK_SIZE;
                sb->y = bc.y+bc.chunk.y*CHUNK_SIZE;
                return;
            }
            else if(!sb->transparent_block.id)
            {
                sb->transparent_block = b;
                sb->height_transparent = bc.z+bc.chunk.z*CHUNK_SIZE;
            }
        }

        bc.x--;
        bc.y--;
        bc.z--;

        if(bc.x == -1)
        {
            bc.chunk.x --;
            bc.x += CHUNK_SIZE;
        }
        if(bc.y == -1)
        {
            bc.chunk.y --;
            bc.y += CHUNK_SIZE;
        }
        if(bc.z == -1)
        {
            bc.chunk.z --;
            bc.z += CHUNK_SIZE;
        }
    }
}


/*
void Render_Engine::refresh_line_visible2(int x, int y, int z, world_coordonate& jump)
{
    line_presence lp;
    screen_block *sb = projection_grid.get_pos_world(x, y, z);

    sb->block.id = BLOCK_EMPTY;
    sb->transparent_block.id = BLOCK_EMPTY;
    sb->height = 0;
    sb->height_transparent = 0;

    // world.translate_world_view_wposition(x, y, z);

    lp = world.line_visiblePR({x, y, z}, jump);


    sb->block.id = lp.oid;
    sb->height = lp.owcoord.z;
    sb->x = lp.owcoord.x;
    sb->y = lp.owcoord.y;

    sb->transparent_block.id = lp.tid;
    sb->height = lp.twcoord.z;
}
*/

void Render_Engine::refresh_all_block_visible2()
{
    std::cout << "game init : refreshing all block visible (2)... ";
    Uint64 start =  Get_time_ms();

    world_coordonate jump;

    if(world.world_view_position == 0)
        jump = {-1, -1, -1};
    else
    if(world.world_view_position == 1)
        jump = {-1, 1, -1};
    else 
    if(world.world_view_position == 2)
        jump = {1, 1, -1};
    else
    if(world.world_view_position == 3)
        jump = {1, -1, -1};

    for(int x = 0; x < world.max_block_coord.x; x++)
        for(int y = 0; y < world.max_block_coord.y; y++)
            refresh_line_visible2(x, y, world.max_block_coord.z-1, jump);

    for(int x = 0; x < world.max_block_coord.x; x++)
        for(int z = 0; z < world.max_block_coord.z; z++)
            refresh_line_visible2(x, world.max_block_coord.y-1, z, jump);
    
    for(int y = 0; y < world.max_block_coord.y; y++)
        for(int z = 0; z < world.max_block_coord.z; z++)
            refresh_line_visible2(world.max_block_coord.x-1, y, z, jump);

    Uint64 end = Get_time_ms();
    std::cout << "finished !\n";
    std::cout << "\t==> time elapsed : " << end-start << " ms\n";
}

void Render_Engine::set_shadow_context(SDL_Color& render_flags, int x, int y, int z)
{
    Uint32 block_presence = 0;

    if(render_flags.b >= 128)
    {
        block_presence = world.line_presenceF({x+1, y, z+1}, {1, 0, 1});

        if(block_presence%256)
        { 
            render_flags.a |= SHADOW_TOP;
        }
    }

    if(render_flags.g >= 128)
    {
        block_presence = 0;

        if(world.world_view_position == 0)
            block_presence = world.line_presenceF({x+2, y, z+1}, {1, 0, 1});

        if(world.world_view_position == 1)
            block_presence = world.line_presenceF({x+1, y-1, z}, {1, 0, 1});

        if(world.world_view_position == 2)
            block_presence = 1;
        
        if(world.world_view_position == 3)
            block_presence = world.line_presenceF({x+1, y+1, z}, {1, 0, 1});

        if(block_presence%256)
        { 
            render_flags.a |= SHADOW_RIGHT;
        }
    }

    if(render_flags.r >= 128)
    {
        block_presence = 0;
        
        if(world.world_view_position == 0)
            block_presence = world.line_presenceF({x+1, y+1, z}, {1, 0, 1});

        if(world.world_view_position == 1)
            block_presence = world.line_presenceF({x+2, y, z+1}, {1, 0, 1});

        else if(world.world_view_position == 2 && y > 0)
            block_presence = world.line_presenceF({x+1, y-1, z}, {1, 0, 1});
    
        else if(world.world_view_position == 3)
            block_presence = 1;

        if(block_presence%256)
        { 
            render_flags.a |= SHADOW_LEFT;
        }
    }
}

void Render_Engine::set_block_shadow_context2(int face, int i, int j)
{
    screen_block *sb = &projection_grid.pos[face][i][j];

    sb->render_flags.a &= ~(SHADOW_TOP+SHADOW_LEFT);
    sb->render_flags_transparent.a &= ~(SHADOW_TOP+SHADOW_LEFT);

    if(sb->block.id)
    {
        int x = sb->x;
        int y = sb->y;
        int z = sb->height;

        world.translate_world_view_wposition(x, y, z);

        set_shadow_context(sb->render_flags, x, y, z);
    }
    if(sb->transparent_block.id)
    {
        int diff = sb->height_transparent-sb->height;
        int x = sb->x+diff;
        int y = sb->y+diff;
        int z = sb->height_transparent;

        world.translate_world_view_wposition(x, y, z);

        set_shadow_context(sb->render_flags_transparent, x, y, z);
    }
}

void Render_Engine::refresh_all_render_flags2()
{
    std::cout << "render engine : refreshing all render flags (2)... ";
    Uint64 start = Get_time_ms();

    for(int face = 0; face < 3; face++)
    {
        for(int i = 0; i < projection_grid.size[face][0]; i++)
            for(int j = 0; j < projection_grid.size[face][1]; j++)
            {
                screen_block *sb = &projection_grid.pos[face][i][j];
                sb->render_flags = {0, 0, 0, 0};
                sb->render_flags_transparent = {0, 0, 0, 0};
                sb->identical_line_counter = 0;
                sb->identical_line_counter_transparent = 0;

                set_block_renderflags(face, i, j);
                // set_block_shadow_context2(face, i, j);
            }
    }

    for(int face = 0; face < 3; face++)
        for(int i = 0; i < projection_grid.size[face][0]; i++)
            for(int j = 0; j < projection_grid.size[face][1]; j++)
            {
                set_block_shadow_context2(face, i, j);
            }

    Uint64 end = Get_time_ms();
    std::cout << "finished !\n";
    std::cout << "\t==> time elapsed : " << end-start << " ms\n";
}
