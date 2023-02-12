#include <game.hpp>

// bouger ça autre part
int    i32round(double x){return floor(x+0.5);}
short  i16round(double x){return floor(x+0.5);}
Uint16 ui16round(double x){return floor(x+0.5);}
/////////////////////////////

Uint64 Get_time_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Render_Engine::Render_Engine(struct World& _world) : world{_world}
{ 
    world = _world;
    grid_enable = false;
    shader_enable = true;
    SecondaryThread = NULL;

    highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

    refresh_shaders();
}

void Render_Engine::refresh_shaders()
{
    DFIB_shader.reset();
    world_render_shader.reset();
    post_process_shader.reset();
    light_shader.reset();

    std::string geom = "shader/blocks.geom";
    DFIB_shader.load("shader/blocks.vert", "shader/blocks/DFIB.frag", &geom);
    light_shader.load("shader/light.vert", "shader/light.frag", &geom);
    transparent_shader.load("shader/blocks.vert", "shader/blocks/transparent.frag", &geom);

    world_render_shader.load("shader/opaque_world.vert", "shader/opaque_world.frag", NULL);
    post_process_shader.load("shader/post process.vert", "shader/post process.frag", NULL);

    shader_features = 0;
    shader_features ^= SFEATURE_GLOBAL_ILLUMINATION;
    shader_features ^= SFEATURE_AMBIANT_OCCLUSION;
    shader_features ^= SFEATURE_BLOCK_BORDERS;
    shader_features ^= SFEATURE_SHADOWS;
    shader_features ^= SFEATURE_BLOOM;

    float light_direction[3] = {0.75, 0.5, 1};
    GPU_SetUniformfv(3, 3, 1, light_direction);
    float global_illumination[4] = {1, 1, 1, 0.60};
    GPU_SetUniformfv(4, 4, 1, global_illumination);
}

void Render_Engine::refresh_sprite_size()
{
    // std::cout << Textures[BLOCK_BLUE]->src.w;
    block_onscreen_size    = BLOCK_TEXTURE_SIZE*window.scale;
    block_onscreen_half    = BLOCK_TEXTURE_SIZE*window.scale/2.0;
    block_onscreen_quarter = BLOCK_TEXTURE_SIZE*window.scale/4.0;
}

void Render_Engine::render_grid()
{
    // SDL_SetRenderDrawColor(renderer, 255, 255, 0, 127);

    SDL_Color linecolor = {255, 255, 0, 255};

    pixel_coord A = {i32round(target.x - window.size.x*0.50),
                     i32round(target.y + window.size.x*0.25)};

    pixel_coord B = {i32round(A.x + window.size.x*2),
                     i32round(A.y - window.size.x)};

    while(B.x < window.size.x)
    {
        B.x += window.size.x*2;
        B.y -= window.size.x;
    }

    while(A.y >= 0)
    {
        B.y = i32round(B.y-block_onscreen_half);
        A.y = i32round(A.y-block_onscreen_half);
    }

    while(A.x >= 0)
    {
        B.x = i32round(B.x-block_onscreen_size);
        A.x = i32round(A.x-block_onscreen_size);
    }

    while(B.y < window.size.y)
    {
        // SDL_RenderDrawLine(renderer, A.x, A.y, B.x, B.y);
        GPU_Line(screen, A.x, A.y, B.x, B.y, linecolor);

        B.y = i32round(B.y+block_onscreen_half);
        A.y = i32round(A.y+block_onscreen_half);
    }

    pixel_coord C = {i32round(target.x - window.size.x*0.50),
                     i32round(target.y - window.size.x*0.25)};
    
    pixel_coord D = {i32round(C.x + window.size.x*2),
                     i32round(C.y + window.size.x)};

    while(D.x <= window.size.x)
    {
        D.x += window.size.x*2;
        D.y += window.size.x;
    }

    while(D.y >= 0)
    {
        C.y = i32round(C.y-block_onscreen_half);
        D.y = i32round(D.y-block_onscreen_half);
    }

    while(C.x >= 0)
    {
        C.x = i32round(C.x-block_onscreen_size);
        D.x = i32round(D.x-block_onscreen_size);
    }


    while(C.y <= window.size.y)
    {
        GPU_Line(screen, C.x, C.y, D.x, D.y, linecolor);

        D.y = i32round(D.y+block_onscreen_half);
        C.y = i32round(C.y+block_onscreen_half);
    }
}

void Render_Engine::set_block_renderflags(char face, int i, int j)
{
    // r = render flag left
    // g = render flag right
    // b = render flag top
    // a = part culling
    screen_block *sb = projection_grid.get_pos(face, i , j);

    if(!sb)
    {
        std::cout
        << "Can't find matched coord for set_block_renderflags at "
        << (int)face << ' ' << i << ' ' << j << '\n';
        return;
    }

    block_coordonate bc = world.convert_wcoord(sb->x, sb->y, sb->height);
    chunk_coordonate coord = bc.chunk;
    int x = bc.x;
    int y = bc.y;
    int z = bc.z;

    if(sb->transparent_block.id)
    {
        block_coordonate bc = world.convert_wcoord(sb->x_transparent, sb->y_transparent, sb->height_transparent);

        sb->render_flags_transparent.r = world.get_block_id(bc.chunk, bc.x, bc.y+1, bc.z) ? 0 : 128;
        sb->render_flags_transparent.g = world.get_block_id(bc.chunk, bc.x+1, bc.y, bc.z) ? 0 : 128;
        sb->render_flags_transparent.b = world.get_block_id(bc.chunk, bc.x, bc.y, bc.z+1) ? 0 : 128;
        sb->render_flags_transparent.a = 0;
        // Uint8 diff;
        // if(sb->height)
        // {
        //     diff = (sb->height_transparent-sb->height);
        // }
        // else 
        // {
        //     int shift = sb->x_transparent < sb->y_transparent ? sb->x_transparent : sb->y_transparent;
        //     shift = shift < sb->height_transparent ? shift : sb->height_transparent;

        //     diff = shift;
        // }

        // diff = diff > 31 ? 31 : diff;
        // sb->render_flags_transparent.a &= (128+64+32);
        // sb->render_flags_transparent.a |= diff;

        // screen_block *sb_right = projection_grid.get_pos_world(sb->x_transparent, sb->y_transparent-1, sb->height_transparent);
        // if(sb_right)
        // {
        //     diff = (sb->height_transparent-sb_right->height);
        //     diff = diff > 31 ? 31 : diff;
        //     sb->render_flags_transparent.r &= 128;
        //     sb->render_flags_transparent.r |= diff;
        // }

        // screen_block *sb_left = projection_grid.get_pos_world(sb->x_transparent-1, sb->y_transparent, sb->height_transparent);
        // if(sb_left)
        // {
        //     diff = (sb->height_transparent-sb_left->height);
        //     diff = diff > 31 ? 31 : diff;
        //     sb->render_flags_transparent.g &= 128;
        //     sb->render_flags_transparent.g |= diff;
        // }

        // screen_block *sb_down = projection_grid.get_pos_world(sb->x_transparent, sb->y_transparent, sb->height_transparent-1);
        // if(sb_down)
        // {
        //     diff = (sb->height_transparent-sb_down->height);
        //     diff = diff > 31 ? 31 : diff;
        //     sb->render_flags_transparent.b &= 128;
        //     sb->render_flags_transparent.b |= diff;
        // }

        // for borders
        // sb->render_flags_transparent.r += world.get_block_id(coord, tx-1, ty, tz) ? 2 : 0;
        // sb->render_flags_transparent.r += world.get_block_id(coord, tx, ty-1, tz) ? 4 : 0;
        // sb->render_flags_transparent.r += world.get_block_id(coord, tx, ty, tz-1) ? 32 : 0;
    }

    if(!sb->opaque_block.id)
    {
        // std::cout
        // << "Can't find block for set_block_renderflags at "
        // << (int)face << ' ' << i << ' ' << j << '\n';
        return;
    }

    SDL_Color &render_flag = sb->render_flags;
    render_flag.r = 0;
    render_flag.g = 0;
    render_flag.b = 0;

    // for borders
    render_flag.r += world.get_opaque_block_id(coord, x-1, y, z) ? 2 : 0;
    render_flag.r += world.get_opaque_block_id(coord, x, y-1, z) ? 4 : 0;
    render_flag.r += world.get_opaque_block_id(coord, x, y, z-1) ? 32 : 0;

    int left  = world.get_opaque_block_id(coord, x, y+1, z); 
    int right = world.get_opaque_block_id(coord, x+1, y, z);
    int top   = world.get_opaque_block_id(coord, x, y, z+1);

    // t : corner left & l : corner top left
    render_flag.b += !(render_flag.b & 80) && world.get_opaque_block_id(coord, x-1, y+1, z+1) ? 2 : 0;

    // t : corner right & r : corner top right
    render_flag.b += !(render_flag.b & 40) && world.get_opaque_block_id(coord, x+1, y-1, z+1) ? 1 : 0;

    // r : left && l : right
    render_flag.g += world.get_opaque_block_id(coord, x+1, y+1, z) ? 64 : 0;

    // r : bottom left & l : bottom right
    render_flag.g += !(render_flag.g & 72) && world.get_opaque_block_id(coord, x+1, y+1, z-1) ? 1 : 0;

    if(!left)
    {
        render_flag.r += 128;

        // AO LEFT
        render_flag.r += world.get_opaque_block_id(coord, x-1, y+1, z) ? 64 : 0;

        //AO RIGHT
        // render_flag.r += world.get_opaque_block_id(coord, x+1, y+1, z) ? 32 : 0;

        //AO TOP
        render_flag.r += world.get_opaque_block_id(coord, x, y+1, z+1) ? 16 : 0;

        //AO BOTTOM
        render_flag.r += world.get_opaque_block_id(coord, x, y+1, z-1) ? 8 : 0;

        //AO CORNER TOP LEFT
        // render_flag.r += !(render_flag.r & 80) && world.get_opaque_block_id(coord, x-1, y+1, z+1) ? 4 : 0;

        //AO CORNER BOTTOM RIGHT
        // render_flag.r += !(render_flag.r & 40) && world.get_opaque_block_id(coord, x+1, y+1, z-1) ? 2 : 0;

        //AO CORNER BOTTOM LEFT
        render_flag.r += !(render_flag.r & 72) && world.get_opaque_block_id(coord, x-1, y+1, z-1) ? 1 : 0;
    }

    if(!right)
    {
        render_flag.g += 128;

        // AO LEFT
        // render_flag.g += world.get_opaque_block_id(coord, x+1, y+1, z) ? 64 : 0;

        //AO RIGHT
        render_flag.g += world.get_opaque_block_id(coord, x+1, y-1, z) ? 32 : 0;

        //AO TOP
        render_flag.g += world.get_opaque_block_id(coord, x+1, y, z+1) ? 16 : 0;

        //AO BOTTOM
        render_flag.g += world.get_opaque_block_id(coord, x+1, y, z-1) ? 8 : 0;

        //AO CORNER TOP RIGHT
        // render_flag.g += !(render_flag.g & 48) && world.get_opaque_block_id(coord, x+1, y-1, z+1) ? 4 : 0;

        //AO CORNER BOTTOM RIGHT
        render_flag.g += !(render_flag.g & 40) && world.get_opaque_block_id(coord, x+1, y-1, z-1) ? 2 : 0;

        //AO CORNER BOTTOM LEFT
        // render_flag.g += !(render_flag.g & 72) && world.get_opaque_block_id(coord, x+1, y+1, z-1) ? 1 : 0;
    }

    if(!top)
    {
        render_flag.b += 128;

        // AO TOP LEFT
        render_flag.b += world.get_opaque_block_id(coord, x-1, y, z+1) ? 64 : 0;

        // AO TOP RIGHT 
        render_flag.b += world.get_opaque_block_id(coord, x, y-1, z+1) ? 32 : 0;

        // AO BOTTOM LEFT 
        render_flag.b += world.get_opaque_block_id(coord, x, y+1, z+1) ? 16 : 0;

        // AO BOTTOM RIGHT
        render_flag.b += world.get_opaque_block_id(coord, x+1, y, z+1) ? 8 : 0;

        // AO CORNER TOP
        render_flag.b += !(render_flag.b & 96) && world.get_opaque_block_id(coord, x-1, y-1, z+1) ? 4 : 0;

        // AO CORNER LEFT 
        // render_flag.b += !(render_flag.b & 80) && world.get_opaque_block_id(coord, x-1, y+1, z+1) ? 2 : 0;

        // AO CORNER RIGHT
        // render_flag.b += !(render_flag.b & 40) && world.get_opaque_block_id(coord, x+1, y-1, z+1) ? 1 : 0;
    }
}

bool Render_Engine::render_block(const chunk_coordonate &wcoord, const chunk_coordonate &pgcoord, GPU_Rect& src_rect, GPU_Rect& dst_rect)
{
    screen_block *sb = &projection_grid.pos[pgcoord.x][pgcoord.y][pgcoord.z];
    block b = sb->opaque_block;

    if(sb && sb->is_on_screen && b.id)
    {
        sb->render_flags.a &= ~(0b11);
        sb->render_flags.a += pgcoord.x;
        GPU_SetColor(Textures[MOSAIC]->ptr, sb->render_flags);

        // Uint16 id = 255-pgcoord.z;
        Uint16 id = b.id-1;
        // Uint16 id = pgcoord.x+1;
        // Uint16 id = current_block_tmp-1;

        sprite_counter++;

        src_rect.x = id;
        src_rect.y = sb->height;

        src_rect.w = MOSAIC_TEXTURE_SIZE+(256.0*sb->identical_line_counter);

        // dst_rect.x = sb->x;
        // dst_rect.y = sb->y;

        dst_rect.x = wcoord.x;
        dst_rect.y = wcoord.y;

        GPU_BlitRect(
                    Textures[MOSAIC]->ptr,
                    &src_rect,
                    DFIB_screen,
                    &dst_rect
                    );
        
        if(b.id > BLOCK_LIGHT_LIMIT)
        {
            lights.push(sb);
        }

        return true;
    }
    return false;
}

void Render_Engine::render_world()
{    
    int face;
    GPU_Rect src_rect = {0, 0, MOSAIC_TEXTURE_SIZE, MOSAIC_TEXTURE_SIZE};
    GPU_Rect dst_rect = {0, 0, 0, 0};

    face = 0;

    for(int j  = projection_grid.visible_frags[face][1].end; 
            j >= projection_grid.visible_frags[face][1].beg; j--)

    for(int i = projection_grid.visible_frags[face][0].beg;
            i < projection_grid.visible_frags[face][0].end; i++)

            if(render_block({-j, i-j, 0}, {face, i, j}, src_rect, dst_rect))
                i += projection_grid.pos[face][i][j].identical_line_counter;

    face = 1;
    for(int j  = projection_grid.visible_frags[face][1].end; 
            j >= projection_grid.visible_frags[face][1].beg; j--)

        for(int i = projection_grid.visible_frags[face][0].beg;
                i < projection_grid.visible_frags[face][0].end; i++)

            if(render_block({i-j, -j, 0}, {face, i, j}, src_rect, dst_rect))
                i += projection_grid.pos[face][i][j].identical_line_counter;

    face = 2;
    for(int i = projection_grid.visible_frags[face][0].beg;
            i < projection_grid.visible_frags[face][0].end; i++)

        for(int j = projection_grid.visible_frags[face][1].beg;
                j < projection_grid.visible_frags[face][1].end; j++)

            if(render_block({i, j, 0}, {face, i, j}, src_rect, dst_rect))
                j += projection_grid.pos[face][i][j].identical_line_counter;
}

bool Render_Engine::render_transparent_block(const chunk_coordonate &wcoord, const chunk_coordonate &pgcoord, GPU_Rect& src_rect, GPU_Rect& dst_rect)
{
    screen_block *sb = &projection_grid.pos[pgcoord.x][pgcoord.y][pgcoord.z];
    block b = sb->transparent_block;

    if(sb && sb->is_on_screen && b.id)
    {
        // sb->render_flags_transparent.a &= ~(0b11);
        // sb->render_flags_transparent.a += pgcoord.x;
        sb->render_flags_transparent.b &= ~(32);
        sb->render_flags_transparent.g &= ~(32);
        sb->render_flags_transparent.b += pgcoord.x%2 ? 32 : 0;
        sb->render_flags_transparent.g += pgcoord.x&2 ? 32 : 0;

        GPU_SetColor(Textures[MOSAIC]->ptr, sb->render_flags_transparent);

        sprite_counter++;

        src_rect.x = b.id-1;
        src_rect.y = sb->height_transparent;

        src_rect.w = MOSAIC_TEXTURE_SIZE+(256.0*sb->identical_line_counter_transparent);

        dst_rect.x = wcoord.x;
        dst_rect.y = wcoord.y;

        GPU_BlitRect(
                    Textures[MOSAIC]->ptr,
                    &src_rect,
                    transparent_screen,
                    &dst_rect
                    );

        return true;
    }
    return false;
}

void Render_Engine::render_transparent_world()
{    
    int face;
    GPU_Rect src_rect = {0, 0, MOSAIC_TEXTURE_SIZE, MOSAIC_TEXTURE_SIZE};
    GPU_Rect dst_rect = {0, 0, 0, 0};

    face = 0;
    for(int j  = projection_grid.visible_frags[face][1].end; 
            j >= projection_grid.visible_frags[face][1].beg; j--)

    for(int i = projection_grid.visible_frags[face][0].beg;
            i < projection_grid.visible_frags[face][0].end; i++)

            if(render_transparent_block({-j, i-j, 0}, {face, i, j}, src_rect, dst_rect))
                i += projection_grid.pos[face][i][j].identical_line_counter_transparent;

    face = 1;
    for(int j  = projection_grid.visible_frags[face][1].end; 
            j >= projection_grid.visible_frags[face][1].beg; j--)

        for(int i = projection_grid.visible_frags[face][0].beg;
                i < projection_grid.visible_frags[face][0].end; i++)

            if(render_transparent_block({i-j, -j, 0}, {face, i, j}, src_rect, dst_rect))
                i += projection_grid.pos[face][i][j].identical_line_counter_transparent;
            
    face = 2;
    for(int i = projection_grid.visible_frags[face][0].beg;
            i < projection_grid.visible_frags[face][0].end; i++)

        for(int j = projection_grid.visible_frags[face][1].beg;
                j < projection_grid.visible_frags[face][1].end; j++)
            
            if(render_transparent_block({i, j, 0}, {face, i, j}, src_rect, dst_rect))
                j += projection_grid.pos[face][i][j].identical_line_counter_transparent;
}

void Render_Engine::render_highlighted_blocks()
{
    GPU_Rect src_rect = {0, roundf(highlight_wcoord.z), BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE};

    GPU_Rect dst_rect = {roundf(highlight_wcoord.x-highlight_wcoord.z),
                         roundf(highlight_wcoord.y-highlight_wcoord.z),
                         0, 0};

    screen_block *sb = projection_grid.get_pos_world(highlight_wcoord.x, highlight_wcoord.y, highlight_wcoord.z);

    if(sb)
    {
        GPU_SetUniformi(8, BLOCK_TEXTURE_SIZE);


        if(highlight_type >= HIGHLIGHT_FLOOR && highlight_wcoord2.x != HIGHLIGHT_NOCOORD && highlight_wcoord.x != HIGHLIGHT_NOCOORD && highlight_type != HIGHLIGHT_PIPETTE)
        {
            int xbeg = highlight_wcoord.x;
            int xend = highlight_wcoord2.x;

            int ybeg = highlight_wcoord.y;
            int yend = highlight_wcoord2.y;

            int zbeg = highlight_wcoord.z;
            int zend = highlight_wcoord2.z;

            if(highlight_wcoord.x > highlight_wcoord2.x)
            {
                xbeg = highlight_wcoord2.x;
                xend = highlight_wcoord.x;
            }

            if(highlight_wcoord.y > highlight_wcoord2.y)
            {
                ybeg = highlight_wcoord2.y;
                yend = highlight_wcoord.y;
            }

            if(highlight_wcoord.z > highlight_wcoord2.z)
            {
                zbeg = highlight_wcoord2.z;
                zend = highlight_wcoord.z;
            }

            GPU_SetColor(Textures[BLOCK_HIGHLIGHT]->ptr, {128, 128, 128, 0});

            // int xbeg_final = floor((2.0*(target.x - screen->w))/block_onscreen_size -1);
            // int xend_final = floor((2.0*target.x)/block_onscreen_size + 1)+1;

            // set_in_interval(xbeg_final, xbeg, xend);
            // set_in_interval(xend_final, xbeg, xend);


            for(int x = xbeg; x <= xend; x ++)
            for(int y = ybeg; y <= yend; y ++)
            for(int z = zbeg; z <= zend; z ++)
            {
                // if(highlight_type == HIGHLIGHT_VOLUME && 
                //    (x != xend && y != yend && z != zend))
                //     continue;

                if(highlight_type == HIGHLIGHT_VOLUME)
                {
                    int condx = (x != xend && x != xbeg) ? 1 : 0;
                    int condy = (y != yend && y != ybeg) ? 1 : 0;
                    int condz = (z != zend && z != zbeg) ? 1 : 0;
                    int cond = condx + condy + condz;

                    if(cond > 1)
                        continue;
                }

                int xspos = target.x + block_onscreen_half*(x - y - 1);
                if(xspos < -block_onscreen_half || xspos > screen->w+block_onscreen_half)
                    continue;
                
                int yspos = target.y + block_onscreen_quarter*(x + y - 2*z);
                if(yspos < -block_onscreen_half || yspos > screen->h+block_onscreen_half)
                    continue;

                src_rect = {0, roundf(z), BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE};

                dst_rect = {roundf(x-z),
                            roundf(y-z),
                            0, 0};
                
                sb = projection_grid.get_pos_world(x, y, z);

                if(!sb)
                    continue;

                if(sb->height == z)
                {
                    GPU_SetColor(Textures[BLOCK_HIGHLIGHT]->ptr, sb->render_flags);
                    GPU_BlitRect(Textures[BLOCK_HIGHLIGHT]->ptr, &src_rect, transparent_screen, &dst_rect);
                }

                else if(sb->height <= z)
                {
                    GPU_SetColor(Textures[BLOCK_HIGHLIGHT]->ptr, {128, 128, 128, 0});
                    GPU_BlitRect(Textures[BLOCK_HIGHLIGHT]->ptr, &src_rect, transparent_screen, &dst_rect);
                }
            }


        }
        else
        {
            int id = world.get_block_id_wcoord(highlight_wcoord.x+1, highlight_wcoord.y+1, highlight_wcoord.z);

            if(sb->height == highlight_wcoord.z)
            {
                SDL_Color rf = sb->render_flags;

                if(id && id < BLOCK_TRANSPARENT_LIMIT)
                    rf.a |= 1;

                GPU_SetColor(Textures[BLOCK_HIGHLIGHT]->ptr, rf);
            }
            else
            {
                SDL_Color rf = {128, 128, 128, 0};

                int t = world.get_block_id_wcoord(highlight_wcoord.x, highlight_wcoord.y, highlight_wcoord.z+1);
                int l = world.get_block_id_wcoord(highlight_wcoord.x, highlight_wcoord.y+1, highlight_wcoord.z);
                int r = world.get_block_id_wcoord(highlight_wcoord.x+1, highlight_wcoord.y, highlight_wcoord.z);

                if(id && id < BLOCK_TRANSPARENT_LIMIT)
                    rf.a += 1;

                if(t && t < BLOCK_TRANSPARENT_LIMIT)
                    rf.b = 0;

                if(l && l < BLOCK_TRANSPARENT_LIMIT)
                    rf.r = 0;

                if(r && r < BLOCK_TRANSPARENT_LIMIT)
                    rf.g = 0;
                
                GPU_SetColor(Textures[BLOCK_HIGHLIGHT]->ptr, rf);
            }


            GPU_BlitRect(Textures[BLOCK_HIGHLIGHT]->ptr, &src_rect, transparent_screen, &dst_rect);
        }
    }


    GPU_SetUniformi(8, MOSAIC_TEXTURE_SIZE);
}

void Render_Engine::render_lights()
{
    // shader.activate();
    // light_shader.activate();
    // light_shader.deactivate();

    GPU_Rect src_rect = {0, 0, 1024, 1024};
    GPU_Rect dst_rect = {0, 0, 0, 0};

    while(!lights.empty())
    {
        screen_block *sb = lights.front();

        src_rect.x = sb->opaque_block.id-1;
        src_rect.y = sb->height;

        dst_rect.x = sb->x-sb->height;
        dst_rect.y = sb->y-sb->height;

        GPU_SetColor(Textures[BLOCK_LIGHT]->ptr, sb->render_flags);
        GPU_BlitRect(Textures[BLOCK_LIGHT]->ptr, &src_rect, light_screen, &dst_rect);

        lights.pop();
    }

    // GPU_SetUniformf(6, block_onscreen_size);
}

void Render_Engine::refresh_block_visible(const chunk_coordonate& coord, const int x, const int y, const int z)
{
    int wx = x + coord.x*CHUNK_SIZE;
    int wy = y + coord.y*CHUNK_SIZE;
    int wz = z + coord.z*CHUNK_SIZE;

    int shiftx = world.max_block_coord.x-wx-1;
    int shifty = world.max_block_coord.y-wy-1;
    int shiftz = max_height_render-wz-1;

    int shiftmin = shiftx < shifty ? shiftx : shifty;
    shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

    wx += shiftmin;
    wy += shiftmin;
    wz += shiftmin;

    refresh_line_visible2(wx, wy, wz);
}

void Render_Engine::refresh_pg_block_visible()
{
    screen_block *sb;

    int Px_min = floor(-2*target.x/block_onscreen_size -1);
    int Py_min = floor(-4*target.y/block_onscreen_size -2);
    int Px_max = ceil(2*(screen->w-target.x)/block_onscreen_size + 1);
    int Py_max = ceil(4*(screen->h-target.y)/block_onscreen_size + 2);

    int coosumx  = 0;
    int coosumy = 0;

    for(int y = projection_grid.visible_frags[0][0].beg;
            y < projection_grid.visible_frags[0][0].end;
            y++)
        for(int z  = projection_grid.visible_frags[0][1].end; 
                z >= projection_grid.visible_frags[0][1].beg;
                z--)
        {
            sb = &projection_grid.pos[0][y][z];

            coosumx = -y;
            coosumy = y-2*z;

            if(coosumx < Px_min || coosumx > Px_max || coosumy < Py_min || coosumy > Py_max)
                sb->is_on_screen = false;
            else
                sb->is_on_screen = true;
        }


    for(int z  = projection_grid.visible_frags[1][1].end; 
            z >= projection_grid.visible_frags[1][1].beg;
            z--)
        for(int x = projection_grid.visible_frags[1][0].beg;
                x < projection_grid.visible_frags[1][0].end;
                x++)
        {
            sb = &projection_grid.pos[1][x][z];

            coosumx = x;
            coosumy = x-2*z;

            if(coosumx < Px_min || coosumx > Px_max || coosumy < Py_min || coosumy > Py_max)
                sb->is_on_screen = false;
            else
                sb->is_on_screen = true;
        }


    for(int x = projection_grid.visible_frags[2][0].beg;
            x < projection_grid.visible_frags[2][0].end;
            x++)
        for(int y = projection_grid.visible_frags[2][1].beg;
                y < projection_grid.visible_frags[2][1].end;
                y++)
        {
            sb = &projection_grid.pos[2][x][y];

            coosumx = x-y;
            coosumy = x+y;

            if(coosumx < Px_min || coosumx > Px_max || coosumy < Py_min || coosumy > Py_max)
                sb->is_on_screen = false;
            else
                sb->is_on_screen = true;
        }
}

void Render_Engine::refresh_block_render_flags(const chunk_coordonate& coord, const int x, const int y, const int z)
{
    for(char _x = -1; _x <= 1; _x++)
        for(char _y = -1; _y <= 1; _y++)
            for(char _z = -1; _z <= 1; _z++)
            {
                chunk_coordonate c = projection_grid.convert_wcoord(coord.x*CHUNK_SIZE+x+_x, 
                                                                    coord.y*CHUNK_SIZE+y+_y, 
                                                                    coord.z*CHUNK_SIZE+z+_z);

                set_block_renderflags(c.x, c.y, c.z);
            }

    coord3D wcoord = {x + coord.x*CHUNK_SIZE, y + coord.y*CHUNK_SIZE, z + coord.z*CHUNK_SIZE};
    refresh_line_shadows(wcoord, wcoord);

    // refresh_line_shadows(x + coord.x*CHUNK_SIZE, x + coord.x*CHUNK_SIZE, y + coord.y*CHUNK_SIZE, z + coord.z*CHUNK_SIZE);
}

void Render_Engine::render_frame()
{
    sprite_counter = 0;

    GPU_ClearColor(DFIB_screen, {0, 0, 0, 0});
    GPU_ClearColor(Color_screen, {0, 0, 0, 0});
    GPU_ClearColor(transparent_screen, {0, 0, 0, 0});
    GPU_ClearColor(screen,  {105, 156, 203, 255});

    /****************** GENERATING BACKGROUND **************************/
    background_shader->activate();

    GPU_SetUniformf(1, timems);
    int win_const[4] = {screen->w, screen->h, target.x, target.y};
    GPU_SetUniformiv(5, 4, 1, win_const);
    GPU_SetUniformui(17, background_shader_data + (world.world_view_position<<30));

    GPU_SetShaderImage(Textures[BACKGROUND_SUNSET]->ptr, 16, 8); // donne iChannel0

    GPU_BlitRect(Textures[BACKGROUND_SUNSET]->ptr, NULL, Color_screen, NULL);

    background_shader->deactivate();
    /*******************************************************************/
    // GPU_Blit(background_image, NULL, screen, 0, 0); // draw une image de taille identiques à lécran
    // GPU_Blit(background_image, NULL, screen2, 0, 0); 
    // GPU_Blit(background_image, NULL, Color_screen, 0, 0);    


    /****************** SETTING SHADER UNIFORMS ************************/
    DFIB_shader.activate();

    GPU_SetUniformf(1, timems/7500.0);
    GPU_SetUniformi(2, shader_features);
    // GPU_SetUniformfv(3, 3, 1, gi_direction);
    // GPU_SetUniformfv(4, 4, 1, global_illumination);
    GPU_SetUniformiv(5, 4, 1, win_const);
    GPU_SetUniformf(6, block_onscreen_size);
    GPU_SetUniformi(7, BLOCK_TEXTURE_SIZE);
    GPU_SetUniformi(8, MOSAIC_TEXTURE_SIZE);

    GPU_SetShaderImage(DFIB_FBO, DFIB_shader.get_location("DFIB"), 3);
    /******************************************************************/

    /****************** RENDERING THE WORLD ***************************/
    render_world();

    GPU_ClearColor(light_screen, {0, 0, 0, 255});
    light_shader.activate();
    GPU_SetShaderImage(Textures[MOSAIC]->ptr, light_shader.get_location("block_atlas"), 1);
    GPU_SetUniformiv(5, 4, 1, win_const);
    GPU_SetUniformf(6, block_onscreen_size);
    GPU_SetUniformi(8, 1024);
    GPU_SetUniformi(7, 1024);
    GPU_SetUniformf(1, timems/1000.0);
    // GPU_SetShaderImage(Color_FBO, light_shader.get_location("world"), 3);
    render_lights();
    light_shader.deactivate();

    
    world_render_shader.activate();
    GPU_SetShaderImage(light_FBO, world_render_shader.get_location("light"), 7);
    GPU_SetShaderImage(Textures[MOSAIC]->ptr, world_render_shader.get_location("atlas"), 1);
    GPU_SetUniformfv(3, 3, 1, gi_direction);
    GPU_SetUniformfv(4, 4, 1, global_illumination);
    GPU_SetUniformf(6, block_onscreen_size);
    GPU_Blit(DFIB_FBO, NULL, Color_screen, 0, 0);
    world_render_shader.deactivate();


    transparent_shader.activate();
    GPU_SetShaderImage(Textures[MOSAIC]->ptr, world_render_shader.get_location("atlas"), 1);
    GPU_SetShaderImage(Color_FBO, transparent_shader.get_location("world"), 8);
    GPU_SetShaderImage(Textures[BLOCK_NORMAL]->ptr, transparent_shader.get_location("normal"), 6);
    GPU_SetUniformf(1, timems/7500.0);
    GPU_SetUniformfv(3, 3, 1, gi_direction);
    GPU_SetUniformfv(4, 4, 1, global_illumination);
    GPU_SetUniformiv(5, 4, 1, win_const);
    GPU_SetUniformf(6, block_onscreen_size);
    GPU_SetUniformi(7, BLOCK_TEXTURE_SIZE);
    GPU_SetUniformi(8, MOSAIC_TEXTURE_SIZE);
    
    render_transparent_world();

    if(window.scale > PANORAMA_SCALE_THRESHOLD && *state == STATE_CONSTRUCTION)
    {
        if(highlight_mode || highlight_type == HIGHLIGHT_PIPETTE)
            highlight_block2();
        GPU_SetShaderImage(Textures[BLOCK_HIGHLIGHT]->ptr, world_render_shader.get_location("atlas"), 1);
        render_highlighted_blocks();
        // GPU_SetShaderImage(Textures[MOSAIC]->ptr, world_render_shader.get_location("atlas"), 1);
    }

    transparent_shader.deactivate();

    
    GPU_Blit(transparent_FBO, NULL, Color_screen, 0, 0);
    /*******************************************************************/

    /****************** POST PROCESS SHADER ****************************/
    post_process_shader.activate();
    GPU_SetUniformi(2, shader_features);
    GPU_Blit(Color_FBO, NULL, screen, 0, 0);
    GPU_Blit(transparent_FBO, NULL, screen, 0, 0);
    post_process_shader.deactivate();
    /*******************************************************************/

    // GPU_Blit(depth_fbo_image, NULL, screen, 0, 0);

    // GPU_Flip(screen);
    SDL_CondWait(GameEvent->secondary_frame_op_finish, GameEvent->init_cond);
}
