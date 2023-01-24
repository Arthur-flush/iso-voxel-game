#include <game.hpp>

void Render_Engine::highlight_block2()
{
    highlight_wcoord = {-1, -1, -1};

    fcoord2D mouse2;

    mouse2.x = mouse.x-target.x;
    mouse2.y = 2.0*(mouse.y-target.y);

    fcoord2D guess;

    guess.x = round(mouse2.y+mouse2.x)/block_onscreen_size;
    guess.y = round(mouse2.y-mouse2.x)/block_onscreen_size;

    coord2D iguess = {ceil(guess.x), ceil(guess.y)};

    // determine on wich vertical half of the case the cursor is
    long double half_value = (mouse2.x/block_onscreen_size)+(iguess.x+iguess.y)/2.0;
    bool half = round(half_value) != ceil(half_value) ? true : false;

    screen_block *sb = projection_grid.get_pos_world(iguess.x, iguess.y, 0);

    int height = sb ? sb->height : -1;

    if(highlight_mode == HIGHLIGHT_REMOVE)
    {
        screen_block *sb_top = projection_grid.get_pos_world(iguess.x, iguess.y, 1);
        screen_block *sb_toplr = NULL;

        if(half)
            sb_toplr = projection_grid.get_pos_world(iguess.x+1, iguess.y, 1);
        else
            sb_toplr = projection_grid.get_pos_world(iguess.x, iguess.y+1, 1);

        int top_height = sb_top ? sb_top->height : -1;
        int toplr_height = sb_toplr ? sb_toplr->height : -1;

        if(sb && height >= top_height && height >= toplr_height)
            highlight_wcoord = {sb->x, sb->y, sb->height};
        
        else if(sb_toplr && toplr_height >= top_height)
            highlight_wcoord = {sb_toplr->x, sb_toplr->y, sb_toplr->height};
        
        else if(sb_top)
            highlight_wcoord = {sb_top->x, sb_top->y, sb_top->height};
    }

    if(highlight_mode == HIGHLIGHT_PLACE)
    {
        screen_block *sb_top = projection_grid.get_pos_world(iguess.x, iguess.y, 1);
        screen_block *sb_toplr = NULL;

        if(half)
            sb_toplr = projection_grid.get_pos_world(iguess.x+1, iguess.y, 1);
        else
            sb_toplr = projection_grid.get_pos_world(iguess.x, iguess.y+1, 1);

        int top_height = sb_top ? sb_top->height : -1;
        int toplr_height = sb_toplr ? sb_toplr->height : -1;

        if(sb && height >= top_height && height >= toplr_height)
        {
            highlight_wcoord = {sb->x, sb->y, sb->height+1};
        }
        
        else if(sb_toplr && toplr_height >= top_height)
        {
            if(half)
                highlight_wcoord = {sb_toplr->x, sb_toplr->y+1, sb_toplr->height};
            else
                highlight_wcoord = {sb_toplr->x+1, sb_toplr->y, sb_toplr->height};
        }
        
        else if(sb_top)
        {
            if(half)
                highlight_wcoord = {sb_top->x+1, sb_top->y, sb_top->height};
            else
                highlight_wcoord = {sb_top->x, sb_top->y+1, sb_top->height};
        }
    }

    if(highlight_mode == HIGHLIGHT_PLACE_ALT)
    {
        screen_block *sb_top = projection_grid.get_pos_world(iguess.x, iguess.y, 1);
        screen_block *sb_toplr = NULL;

        if(half)
            sb_toplr = projection_grid.get_pos_world(iguess.x+1, iguess.y, 1);
        else
            sb_toplr = projection_grid.get_pos_world(iguess.x, iguess.y+1, 1);

        int top_height = sb_top ? sb_top->height : -1;
        int toplr_height = sb_toplr ? sb_toplr->height : -1;

        if(sb && height >= top_height && height >= toplr_height)
        {
            if(half)
                highlight_wcoord = {sb->x, sb->y-1, sb->height};
            else    
                highlight_wcoord = {sb->x-1, sb->y, sb->height};
        }
        
        else if(sb_toplr && toplr_height >= top_height)
        {
            if(half)
                highlight_wcoord = {sb_toplr->x-1, sb_toplr->y, sb_toplr->height};
            else
                highlight_wcoord = {sb_toplr->x, sb_toplr->y-1, sb_toplr->height};
        }
        
        else if(sb_top)
            highlight_wcoord = {sb_top->x, sb_top->y, sb_top->height-1};

        int id = world.get_block_id_wcoord(highlight_wcoord.x, highlight_wcoord.y, highlight_wcoord.z);

        if(id && id != BLOCK_WATER)
        {
            highlight_wcoord = {1, 1, 1};
        }
    }
}

void Render_Engine::rotate_camera(int new_wvp)
{
    float cx = (screen->w-2*target.x)/block_onscreen_size;
    float cy = (screen->h-2*target.y)/block_onscreen_half;

    float x = (cx+cy)/2;
    float y = (cy-cx)/2;

    int height = 0;

    screen_block *sb = projection_grid.get_pos_world(round(x), round(y), 0);

    if(sb)
        height = sb->height;

    float nx = world.max_block_coord.x - y - height*2;
    float ny = x;

    if(new_wvp == 1)
    {
        nx = world.max_block_coord.x - y - height*2;
        ny = x;  
    }
    else if(new_wvp == -1)
    {
        nx = y;  
        ny = world.max_block_coord.y - x - height*2;
    }

    target.x = screen->w/2 - block_onscreen_half*(nx - ny);
    target.y = screen->h/2 - block_onscreen_quarter*(nx + ny);

    world.world_view_position += new_wvp;

    if(world.world_view_position < 0)
        world.world_view_position = 3;

    else if(world.world_view_position > 3)
        world.world_view_position = 0;

    projection_grid.refresh_visible_frags(target, screen->w, screen->h, block_onscreen_size);
}

void Render_Engine::refresh_pg_MHR()
{
    // std::cout << "render engine : refreshing pg MHR onscreen ... ";
    // Uint64 start = Get_time_ms();

    int face, i, j, diff;
    
    screen_block *sb;

    world_coordonate coord;

    for(face = 0; face < 3; face ++)
    {
        for(i = projection_grid.visible_frags_save[face][0].beg;
            i < projection_grid.visible_frags_save[face][0].end;
            i++)
        {
            for(j = projection_grid.visible_frags_save[face][1].beg;
                j < projection_grid.visible_frags_save[face][1].end;
                j++)
                {
                    sb = &projection_grid.pos[face][i][j];

                    if(sb->height >= max_height_render || sb->height_transparent >= max_height_render)
                    {
                        if(face == 0)
                        {
                            coord.x = 0;
                            coord.y = i;
                            coord.z = j;
                        }
                        else if(face == 1)
                        {
                            coord.x = i;
                            coord.y = 0;
                            coord.z = j;
                        }
                        else if(face == 2)
                        {
                            coord.x = i;
                            coord.y = j;
                            coord.z = 0;
                        }

                        int diffx = world.max_block_coord.x - 1 - coord.x;
                        int diffy = world.max_block_coord.y - 1 - coord.y;
                        int diffz = max_height_render - 1 - coord.z;

                        diff = diffx < diffy ? diffx : diffy;
                        diff = diff < diffz ? diff : diffz;

                        coord.x += diff;
                        coord.y += diff;
                        coord.z += diff;

                        refresh_line_visible2(coord.x, coord.y, coord.z);

                        set_block_renderflags(face, i, j);

                        set_block_shadow_context2(face, i, j);
                    }
            }
        }
    }
    
    projection_grid.refresh_all_identical_line();

    // Uint64 end = Get_time_ms();
    // std::cout << "finished !\n";
    // std::cout << "\t==> time elapsed : " << end-start << " ms\n";
}

void Render_Engine::refresh_pg_onscreen()
{
    // std::cout << "render engine : refreshing pg onscreen (2)... ";
    // Uint64 start = Get_time_ms();

    int face, i, j, diff;

    world_coordonate coord;

    for(face = 0; face < 3; face ++)
    {
        for(i = projection_grid.visible_frags_save[face][0].beg;
            i < projection_grid.visible_frags_save[face][0].end;
            i++)
        {
            for(j = projection_grid.visible_frags_save[face][1].beg;
                j < projection_grid.visible_frags_save[face][1].end;
                j++)
                {
                    if(face == 0)
                    {
                        coord.x = 0;
                        coord.y = i;
                        coord.z = j;
                    }
                    else if(face == 1)
                    {
                        if(i == 0)
                            continue;

                        coord.x = i;
                        coord.y = 0;
                        coord.z = j;
                    }
                    else if(face == 2)
                    {
                        if(i == 0 || j == 0)
                            continue;

                        coord.x = i;
                        coord.y = j;
                        coord.z = 0;
                    }

                    int diffz = max_height_render - 1 - coord.z;
                    if(diffz < 0)
                        continue;

                    int diffx = world.max_block_coord.x - 1 - coord.x;
                    int diffy = world.max_block_coord.y - 1 - coord.y;

                    diff = diffx < diffy ? diffx : diffy;
                    diff = diff < diffz ? diff : diffz;

                    coord.x += diff;
                    coord.y += diff;
                    coord.z += diff;

                    refresh_line_visible2(coord.x, coord.y, coord.z);

                    set_block_renderflags(face, i, j);

                    set_block_shadow_context2(face, i, j);
            }
        }
    }
    
    projection_grid.refresh_all_identical_line();

    // Uint64 end = Get_time_ms();
    // std::cout << "finished !\n";
    // std::cout << "\t==> time elapsed : " << end-start << " ms\n";
}

void Render_Engine::refresh_line_shadows(int x)
{
    chunk_coordonate pgcoord;

    for(int y = 0; y <= world.max_block_coord.y; y++)
        for(int z = 0; z <= world.max_block_coord.z; z++)
        {
            pgcoord = projection_grid.convert_wcoord(x, y, z);

            if(pgcoord.x != -1 && pgcoord.y != -1 && pgcoord.z != -1)
            {
                set_block_shadow_context2(pgcoord.x, pgcoord.y, pgcoord.z);
            }
        }
}

void Render_Engine::refresh_line_visible2(int x, int y, int z)
{
    block b;
    screen_block *sb = projection_grid.get_pos_world(x, y, z);

    sb->transparent_block.id = BLOCK_EMPTY;
    sb->height_transparent = 0;

    sb->block.id = BLOCK_EMPTY;
    sb->height = 0;
    sb->x = 0;
    sb->y = 0;
    
    block_coordonate bc = world.convert_wcoord(x, y, z);
    block_coordonate bc2;

    struct chunk* c;
    bool new_chunk = true;

    while(bc.chunk.x >= 0 && bc.chunk.y >= 0 && bc.chunk.z >= 0)
    {
        bc2 = bc;

        // Exchange x and y, then reverse y
        if(world.world_view_position == 1)
        {
            bc2.y = bc.x;
            bc2.x = bc.y;

            bc2.chunk.y = bc.chunk.x;
            bc2.chunk.x = bc.chunk.y;

            bc2.chunk.y = world.max_chunk_coord.y-bc2.chunk.y;
            bc2.y = CHUNK_SIZE-1-bc2.y;
        }
        // Exchange x and y, then reverse x
        else if(world.world_view_position == 3)
        {
            bc2.y = bc.x;
            bc2.x = bc.y;

            bc2.chunk.y = bc.chunk.x;
            bc2.chunk.x = bc.chunk.y;

            bc2.chunk.x = world.max_chunk_coord.x-bc2.chunk.x;
            bc2.x = CHUNK_SIZE-1-bc2.x;
        }
        // Reverse both x and y
        else if(world.world_view_position == 2)
        {
            bc2.chunk.y = world.max_chunk_coord.y-bc2.chunk.y;
            bc2.y = CHUNK_SIZE-1-bc2.y;
        
            bc2.chunk.x = world.max_chunk_coord.x-bc2.chunk.x;
            bc2.x = CHUNK_SIZE-1-bc2.x;
        }

        if(new_chunk)
        {
            c = &world.chunk[bc2.chunk.x][bc2.chunk.y][bc2.chunk.z];

            new_chunk = false;

            if(c->compress_value != CHUNK_NON_UNIFORM)
            {

                if(c->compress_value != CHUNK_EMPTY)
                {
                    if(c->compress_value < BLOCK_TRANSPARENT_LIMIT)
                    {
                        sb->block.id = c->compress_value;

                        sb->height = bc.z+bc.chunk.z*CHUNK_SIZE;
                        sb->x = bc.x+bc.chunk.x*CHUNK_SIZE;
                        sb->y = bc.y+bc.chunk.y*CHUNK_SIZE;
                        return;
                    }
                    else if(!sb->transparent_block.id)
                    {
                        sb->transparent_block.id = c->compress_value;
                        sb->height_transparent = bc.z+bc.chunk.z*CHUNK_SIZE;
                    }
                }

                int min = bc.x < bc.y ? bc.x : bc.y;
                min = min < bc.z ? min : bc.z;

                bc.x -= min+1;
                bc.y -= min+1;
                bc.z -= min+1;

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

                new_chunk = true;
            }
        }

        else
        {
            b.id = c->block[bc2.x][bc2.y][bc2.z].id;

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
                    sb->transparent_block.id = b.id;
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
                new_chunk = true;
            }
            if(bc.y == -1)
            {
                bc.chunk.y --;
                bc.y += CHUNK_SIZE;
                new_chunk = true;
            }
            if(bc.z == -1)
            {
                bc.chunk.z --;
                bc.z += CHUNK_SIZE;
                new_chunk = true;
            }
        }
    }
}

void Render_Engine::refresh_all_block_visible2()
{
    // std::cout << "game init : refreshing all block visible (2)... ";
    // Uint64 start =  Get_time_ms();

    for(int x = 0; x < world.max_block_coord.x; x++)
        for(int z = max_height_render; z < world.max_block_coord.z; z++)
        {
            screen_block * sb = projection_grid.get_pos_world(x, 0, z);

            sb->block.id = 0;
            sb->transparent_block.id = 0;
        }

    for(int y = 0; y < world.max_block_coord.y; y++)
        for(int z = max_height_render; z < world.max_block_coord.z; z++)
        {
            screen_block * sb = projection_grid.get_pos_world(0, y, z);

            sb->block.id = 0;
            sb->transparent_block.id = 0;
        }

    for(int x = 0; x < world.max_block_coord.x; x++)
        for(int y = 0; y < world.max_block_coord.y; y++)
            refresh_line_visible2(x, y, max_height_render-1);

    for(int x = 0; x < world.max_block_coord.x; x++)
        for(int z = 0; z < max_height_render; z++)
            refresh_line_visible2(x, world.max_block_coord.y-1, z);
    
    for(int y = 0; y < world.max_block_coord.y; y++)
        for(int z = 0; z < max_height_render; z++)
            refresh_line_visible2(world.max_block_coord.x-1, y, z);

    // Uint64 end = Get_time_ms();
    // std::cout << "finished !\n";
    // std::cout << "\t==> time elapsed : " << end-start << " ms\n";
}

void Render_Engine::set_shadow_context(SDL_Color& render_flags, int x, int y, int z)
{
    Uint32 block_presence = 0;

    if(render_flags.b >= 128)
    {
        block_presence = world.shadow_caster_presence({x+1, y, z+1});

        if(block_presence%256)
        { 
            render_flags.a |= SHADOW_TOP;
        }
    }

    if(render_flags.g >= 128)
    {
        block_presence = 0;

        if(world.world_view_position == 0)
            block_presence = world.shadow_caster_presence({x+2, y, z+1});

        if(world.world_view_position == 1)
            block_presence = world.shadow_caster_presence({x+1, y-1, z});

        if(world.world_view_position == 2)
            block_presence = 1;
        
        if(world.world_view_position == 3)
            block_presence = world.shadow_caster_presence({x+1, y+1, z});

        if(block_presence%256)
        { 
            render_flags.a |= SHADOW_RIGHT;
        }
    }

    if(render_flags.r >= 128)
    {
        block_presence = 0;
        
        if(world.world_view_position == 0)
            block_presence = world.shadow_caster_presence({x+1, y+1, z});

        if(world.world_view_position == 1)
            block_presence = world.shadow_caster_presence({x+2, y, z+1});

        else if(world.world_view_position == 2 && y > 0)
            block_presence = world.shadow_caster_presence({x+1, y-1, z});
    
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

    sb->render_flags.a &= 0b00011111;
    sb->render_flags_transparent.a &= 0b00011111;

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
    // std::cout << "render engine : refreshing all render flags (2)... ";
    // Uint64 start = Get_time_ms();

    for(int face = 0; face < 3; face++)
    {
        for(int i = 0; i < projection_grid.size[face][0]; i++)
            for(int j = 0; j < projection_grid.size[face][1]; j++)
            {
                // screen_block *sb = &projection_grid.pos[face][i][j];
                
                // sb->render_flags = {0, 0, 0, 0};
                // sb->render_flags_transparent = {0, 0, 0, 0};
                // sb->identical_line_counter = 0;
                // sb->identical_line_counter_transparent = 0;
                
                set_block_renderflags(face, i, j);
                set_block_shadow_context2(face, i, j);
            }
    }

    // Uint64 end = Get_time_ms();
    // std::cout << "finished !\n";
    // std::cout << "\t==> time elapsed : " << end-start << " ms\n";
}
