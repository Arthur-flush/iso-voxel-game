#include <game.hpp>

int iceil(float x)
{
    return ceil(x);
}

coord3D select_screen_block_coord(screen_block *sb)
{
    if(!sb)
        return {-1, -1, -1};

    if(sb->height_transparent > sb->height && sb->transparent_block.id != BLOCK_WATER)
        return {sb->x_transparent, sb->y_transparent, sb->height_transparent};

    return {sb->x, sb->y, sb->height};
}

void Render_Engine::center_camera()
{
    target.x = screen->w/2 - block_onscreen_half*(world.max_block_coord.x/2 - world.max_block_coord.y/2);
    target.y = screen->h/2 - block_onscreen_quarter*(world.max_block_coord.x/2 + world.max_block_coord.y/2);
}

void Render_Engine::highlight_block2()
{
    highlight_wcoord = {-1, -1, -1};

    fcoord2D mouse2;

    mouse2.x = mouse.x-target.x;
    mouse2.y = 2.0*(mouse.y-target.y);

    fcoord2D guess;

    guess.x = round(mouse2.y+mouse2.x)/block_onscreen_size;
    guess.y = round(mouse2.y-mouse2.x)/block_onscreen_size;


    coord2D iguess = {iceil(guess.x), iceil(guess.y)};

    // determine on wich vertical half of the case the cursor is
    long double half_value = (mouse2.x/block_onscreen_size)+(iguess.x+iguess.y)/2.0;
    bool half = round(half_value) != ceil(half_value) ? true : false;

    screen_block *sb = projection_grid.get_pos_world(iguess.x, iguess.y, 0);

    coord3D csb = select_screen_block_coord(sb);

    if(highlight_mode == HIGHLIGHT_MOD_DELETE || highlight_mode == HIGHLIGHT_MOD_REPLACE)
    {
        screen_block *sb_top = projection_grid.get_pos_world(iguess.x, iguess.y, 1);
        screen_block *sb_toplr = NULL;

        if(half)
            sb_toplr = projection_grid.get_pos_world(iguess.x+1, iguess.y, 1);
        else
            sb_toplr = projection_grid.get_pos_world(iguess.x, iguess.y+1, 1);

        coord3D csb_top = select_screen_block_coord(sb_top);
        coord3D csb_toplr = select_screen_block_coord(sb_toplr);

        if(sb && csb.z >= csb_top.z && csb.z >= csb_toplr.z)
            highlight_wcoord = csb;
        
        else if(sb_toplr && csb_toplr.z >= csb_top.z)
            highlight_wcoord = csb_toplr;
        
        else if(sb_top)
            highlight_wcoord = csb_top;
    }

    if(highlight_mode == HIGHLIGHT_MOD_PLACE)
    {
        screen_block *sb_top = projection_grid.get_pos_world(iguess.x, iguess.y, 1);
        screen_block *sb_toplr = NULL;

        if(half)
            sb_toplr = projection_grid.get_pos_world(iguess.x+1, iguess.y, 1);
        else
            sb_toplr = projection_grid.get_pos_world(iguess.x, iguess.y+1, 1);

        coord3D csb_top = select_screen_block_coord(sb_top);
        coord3D csb_toplr = select_screen_block_coord(sb_toplr);

        if(sb && csb.z >= csb_top.z && csb.z >= csb_toplr.z)
        {
            csb.z ++;
            highlight_wcoord = csb;
        }
        
        else if(sb_toplr && csb_toplr.z >= csb_top.z)
        {
            if(half)
            {
                csb_toplr.y ++;
                highlight_wcoord = csb_toplr;
            }
            else
            {
                csb_toplr.x ++;
                highlight_wcoord = csb_toplr;
            }

        }
        
        else if(sb_top)
        {
            if(half)
            {
                csb_top.x++;
                highlight_wcoord = csb_top;
            }
            else
            {
                csb_top.y++;
                highlight_wcoord = csb_top;
            }
        }
    }

    if(highlight_mode == HIGHLIGHT_MOD_PLACE_ALT)
    {
        screen_block *sb_top = projection_grid.get_pos_world(iguess.x, iguess.y, 1);
        screen_block *sb_toplr = NULL;

        if(half)
            sb_toplr = projection_grid.get_pos_world(iguess.x+1, iguess.y, 1);
        else
            sb_toplr = projection_grid.get_pos_world(iguess.x, iguess.y+1, 1);

        coord3D csb_top = select_screen_block_coord(sb_top);
        coord3D csb_toplr = select_screen_block_coord(sb_toplr);

        if(sb && csb.z >= csb_top.z && csb.z >= csb_toplr.z)
        {
            if(half)
                csb.y--;
            else    
                csb.x--;

            highlight_wcoord = csb;
        }
        
        else if(sb_toplr && csb_toplr.z >= csb_top.z)
        {
            if(half)
                csb_toplr.x--;
            else
                csb_toplr.y--;

            highlight_wcoord = csb_toplr;
        }
        
        else if(sb_top)
        {
            csb_top.z--;
            highlight_wcoord = csb_top;
        }

        int id = world.get_block_id_wcoord(highlight_wcoord.x, highlight_wcoord.y, highlight_wcoord.z);

        if(id && id != BLOCK_WATER)
        {
            highlight_wcoord = {1, 1, 1};
        }
    }

    if(highlight_type == HIGHLIGHT_FLOOR && highlight_wcoord2.z != -1)
    {
        highlight_wcoord = {iguess.x+highlight_wcoord2.z, iguess.y+highlight_wcoord2.z, highlight_wcoord2.z};
    }

    if(highlight_type == HIGHLIGHT_WALL_X && highlight_wcoord2.z != -1)
    {
        int diff = highlight_wcoord2.x - guess.x;

        highlight_wcoord = {iguess.x+diff, iguess.y+diff, diff};
    }

    if(highlight_type == HIGHLIGHT_WALL_Y && highlight_wcoord2.z != -1)
    {
        int diff = highlight_wcoord2.y - guess.y;

        highlight_wcoord = {iguess.x+diff, iguess.y+diff, diff};
    }

    // if( highlight_wcoord.x < 0 ||
    //     highlight_wcoord.y < 0 ||
    //     highlight_wcoord.z < 0 ||
    //     highlight_wcoord.x >= world.max_block_coord.x ||
    //     highlight_wcoord.y >= world.max_block_coord.y ||
    //     highlight_wcoord.z >= world.max_block_coord.z)
    //     highlight_wcoord = {-1, -1, -1};

    set_in_interval(highlight_wcoord.x, 0, world.max_block_coord.x-1);
    set_in_interval(highlight_wcoord.y, 0, world.max_block_coord.y-1);
    set_in_interval(highlight_wcoord.z, 0, world.max_block_coord.z-1);
}

void Render_Engine::set_global_illumination(float gi[4])
{
    global_illumination[0] = gi[0];
    global_illumination[1] = gi[1];
    global_illumination[2] = gi[2];
    global_illumination[3] = gi[3];

    set_global_illumination_direction();
}

void Render_Engine::set_global_illumination_direction()
{
    switch (world.world_view_position)
    {
    case 0:
        gi_direction[0] = 0.75;
        gi_direction[1] = 0.5;
        gi_direction[2] = 1;
        break;

    case 1:
        gi_direction[0] = 0.5;
        gi_direction[1] = 0.75;
        gi_direction[2] = 1;
        break;

    case 2:
        gi_direction[0] = 0.4;
        gi_direction[1] = 0.5;
        gi_direction[2] = 1;
        break;

    case 3:
        gi_direction[0] = 0.5;
        gi_direction[1] = 0.4;
        gi_direction[2] = 1;
        break;

    default:
        break;
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

        if(highlight_wcoord2.x != -1)
        {
            int tmp = highlight_wcoord2.x; 

            highlight_wcoord2.x =  world.max_block_coord.x - highlight_wcoord2.y;
            highlight_wcoord2.y = tmp;
        }
    }
    else if(new_wvp == -1)
    {
        nx = y;  
        ny = world.max_block_coord.y - x - height*2;

        if(highlight_wcoord2.x != -1)
        {
            int tmp = highlight_wcoord2.y; 

            highlight_wcoord2.y =  world.max_block_coord.y - highlight_wcoord2.x;
            highlight_wcoord2.x = tmp;
        }
    }

    target.x = screen->w/2 - block_onscreen_half*(nx - ny);
    target.y = screen->h/2 - block_onscreen_quarter*(nx + ny);

    world.world_view_position += new_wvp;

    if(world.world_view_position < 0)
        world.world_view_position = 3;

    else if(world.world_view_position > 3)
        world.world_view_position = 0;

    projection_grid.refresh_visible_frags(target, screen->w, screen->h, block_onscreen_size);

    set_global_illumination_direction();
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

void Render_Engine::refresh_line_shadows(coord3D beg, coord3D end)
{
    world.translate_world_view_wposition(beg.x, beg.y, beg.z);

    world.translate_world_view_wposition(end.x, end.y, end.z);

    int xbeg = beg.x;
    int xend = end.x;

    int ybeg = beg.y;
    int yend = end.y;

    int zbeg = beg.z;
    int zend = end.z;

    if(beg.x > end.x)
    {
        xbeg = end.x;
        xend = beg.x;
    }

    if(beg.y > end.y)
    {
        ybeg = end.y;
        yend = beg.y;
    }

    if(beg.z > end.z)
    {
        zbeg = end.z;
        zend = beg.z;
    }

    xbeg = 0;
    xbeg -= zend;
    xbeg = xbeg < 0 ? 0 : xbeg;
    
    zbeg = 0;
    // zbeg --;
    // zbeg = zbeg < 0 ? 0 : zbeg;

    // std::cout << "\n" << xbeg << " " << xend << "\n";
    // std::cout << ybeg << " " << yend << "\n";
    // std::cout << zbeg << " " << zend << "\n";

    chunk_coordonate pgcoord;

    for(int x = xbeg; x <= xend; x ++)
    for(int y = ybeg; y <= yend; y ++)
    for(int z = zbeg; z <= zend; z ++)
    {
        int x2 = x;
        int y2 = y;
        int z2 = z;


        world.invert_wvp(x2, y2);

        pgcoord = projection_grid.convert_wcoord(x2, y2, z2);

        if(pgcoord.x != -1)
        {
            set_block_shadow_context2(pgcoord.x, pgcoord.y, pgcoord.z);
        }
    }
}

void Render_Engine::refresh_line_shadows(int xmin, int x, int y, int z)
{
    chunk_coordonate pgcoord;
    // int ytmp = y;

    world.translate_world_view_wposition(x, y, z);
    // world.translate_world_view_wposition(xmin, ytmp, z);

    int xbeg = xmin;
    // int xbeg = xmin-z;
    // xbeg = xbeg < 0 ? 0 : xbeg;

    // std::cout << "xyz : " << x << " " << y << " " << z << "\n";
    // std::cout << "xmin ytmp z : " << xmin << " " << ytmp << " " << z << "\n";
    // std::cout << "xbeg " << xbeg << "\n";

    // int itcounter = 0;

    for(int _x = xbeg; _x <= x; _x++)
        for(int _z = 0; _z <= z; _z++)
        {
            int x2 = _x;
            int y2 = y;
            int z2 = _z;

            // world.translate_world_view_wposition(x2, y2, z2);
            
            world.invert_wvp(x2, y2);

            pgcoord = projection_grid.convert_wcoord(x2, y2, z2);

            if(pgcoord.x != -1 && pgcoord.y != -1 && pgcoord.z != -1)
            {
                // itcounter++;
                set_block_shadow_context2(pgcoord.x, pgcoord.y, pgcoord.z);
            }
        }
    
    // std::cout << itcounter << "\n";
}

void Render_Engine::refresh_line_visible2(int x, int y, int z)
{
    block b;
    screen_block *sb = projection_grid.get_pos_world(x, y, z);

    sb->transparent_block.id = BLOCK_EMPTY;
    sb->height_transparent = 0;

    sb->opaque_block.id = BLOCK_EMPTY;
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
            c = &world.chunks[bc2.chunk.x][bc2.chunk.y][bc2.chunk.z];

            new_chunk = false;

            if(c->compress_value != CHUNK_NON_UNIFORM)
            {

                if(c->compress_value != CHUNK_EMPTY)
                {
                    if(c->compress_value < BLOCK_TRANSPARENT_LIMIT)
                    {
                        sb->opaque_block.id = c->compress_value;

                        sb->height = bc.z+bc.chunk.z*CHUNK_SIZE;
                        sb->x = bc.x+bc.chunk.x*CHUNK_SIZE;
                        sb->y = bc.y+bc.chunk.y*CHUNK_SIZE;
                        return;
                    }
                    else if(!sb->transparent_block.id)
                    {
                        sb->transparent_block.id = c->compress_value;
                        sb->x_transparent = bc.x+bc.chunk.x*CHUNK_SIZE;
                        sb->y_transparent = bc.y+bc.chunk.y*CHUNK_SIZE;
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
            b.id = c->blocks[bc2.x][bc2.y][bc2.z].id;

            if(b.id)
            {
                if(b.id < BLOCK_TRANSPARENT_LIMIT)
                {
                    sb->opaque_block.id = b.id;

                    sb->height = bc.z+bc.chunk.z*CHUNK_SIZE;
                    sb->x = bc.x+bc.chunk.x*CHUNK_SIZE;
                    sb->y = bc.y+bc.chunk.y*CHUNK_SIZE;
                    return;
                }
                else if(!sb->transparent_block.id)
                {
                    sb->transparent_block.id = b.id;
                    sb->x_transparent = bc.x+bc.chunk.x*CHUNK_SIZE;
                    sb->y_transparent = bc.y+bc.chunk.y*CHUNK_SIZE;
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

            sb->opaque_block.id = 0;
            sb->transparent_block.id = 0;
        }

    for(int y = 0; y < world.max_block_coord.y; y++)
        for(int z = max_height_render; z < world.max_block_coord.z; z++)
        {
            screen_block * sb = projection_grid.get_pos_world(0, y, z);

            sb->opaque_block.id = 0;
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

            // render_flags.a += (block_presence/256)-BLOCK_TRANSPARENT_LIMIT;
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

            // render_flags.a += (block_presence/256)-BLOCK_TRANSPARENT_LIMIT;
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

            // render_flags.a += (block_presence/256)-BLOCK_TRANSPARENT_LIMIT;
        }
    }
}

void Render_Engine::set_block_shadow_context2(int face, int i, int j)
{
    screen_block *sb = &projection_grid.pos[face][i][j];

    sb->render_flags.a &= 0b00011111;
    sb->render_flags_transparent.a &= 0b00011111;

    // sb->render_flags.a = 0;
    // sb->render_flags_transparent.a = 0;

    if(sb->opaque_block.id)
    {
        int x = sb->x;
        int y = sb->y;
        int z = sb->height;

        world.translate_world_view_wposition(x, y, z);

        set_shadow_context(sb->render_flags, x, y, z);
    }
    if(sb->transparent_block.id)
    {
        int x = sb->x_transparent;
        int y = sb->y_transparent;
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
