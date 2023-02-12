#include <game.hpp>

std::list<int>::iterator circularPrev(std::list<int> &l, std::list<int>::iterator &it)
{
    return it == l.begin() ? std::prev(l.end()) : std::prev(it);
}

std::list<int>::iterator circularNext(std::list<int> &l, std::list<int>::iterator &it)
{
    return next(it) == l.end() ? l.begin() : std::next(it);
}

pixel_coord mouse = {0};

Game::Game(GPU_Target* _screen) : RE(world), GameEvent(RE)
{
    state = STATE_QUIT;
    init(_screen);
}

void Game::init_Render_Engine(GPU_Target* _screen)
{
    // GPU_AddDepthBuffer(_screen);
    // GPU_SetDepthFunction(_screen, GPU_LEQUAL);

    RE.state = &state;

    RE.highlight_mode = HIGHLIGHT_MOD_NONE;
    RE.screen = _screen;
    RE.window.size.x = DEFAULT_WINDOWS_W;
    RE.window.size.y = DEFAULT_WINDOWS_H;
    RE.GameEvent = &GameEvent;

    RE.window.scale = DEFAULT_SCALE;

    RE.world = world;

    RE.center_camera();
    RE.window.scale = DEFAULT_SCALE;
    RE.refresh_sprite_size();

    RE.max_render_coord.z = (world.max_chunk_coord.z+1)*CHUNK_SIZE;

    RE.DFIB_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.DFIB_FBO, 0, 0);
    RE.DFIB_screen = GPU_LoadTarget(RE.DFIB_FBO);

    RE.Color_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.Color_FBO, 0, 0);
    RE.Color_screen = GPU_LoadTarget(RE.Color_FBO);

    RE.light_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.light_FBO, 0, 0);
    RE.light_screen = GPU_LoadTarget(RE.light_FBO);

    // GPU_SetBlendMode(RE.light_FBO, GPU_BLEND_ADD);

    RE.transparent_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.transparent_FBO, 0, 0);
    RE.transparent_screen = GPU_LoadTarget(RE.transparent_FBO);

    UI.UI_image = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(UI.UI_image, 0, 0);
    UI.UI = GPU_LoadTarget(UI.UI_image);

    RE.highlight_mode = HIGHLIGHT_MOD_NONE;
    RE.highlight_type = HIGHLIGHT_BLOCKS;

    RE.DFIB_shader.activate();
    GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.DFIB_shader.get_location("ao"), 2);
    GPU_SetShaderImage(RE.Textures[BLOCK_BORDER]->ptr, RE.DFIB_shader.get_location("border"), 5);
    GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.DFIB_shader.get_location("normal"), 6);
    RE.DFIB_shader.deactivate();

    RE.transparent_shader.activate();
    GPU_SetShaderImage(RE.Textures[SHADERTEXT_WATER]->ptr, RE.transparent_shader.get_location("water"), 4);
    // GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.transparent_shader.get_location("normal"), 6);
    RE.transparent_shader.deactivate();

    RE.max_height_render = world.max_block_coord.z;

    GPU_AddDepthBuffer(RE.DFIB_screen);
    GPU_SetDepthFunction(RE.DFIB_screen, GPU_LEQUAL);

    GPU_AddDepthBuffer(RE.transparent_screen);
    GPU_SetDepthFunction(RE.transparent_screen, GPU_LEQUAL);

    GPU_AddDepthBuffer(RE.Color_screen);
    GPU_SetDepthFunction(RE.Color_screen, GPU_ALWAYS);
}

void Game::generate_debug_world()
{
    // chunk_coordonate world_size = {16, 16, 16}; // hut
    chunk_coordonate world_size = {16, 16, 1}; // plain
    // chunk_coordonate world_size = {64, 64, 32}; // island
    // chunk_coordonate world_size = {64, 64, 32}; // playground

    // chunk_coordonate world_size = {512, 512, 32};
    // chunk_coordonate world_size = {256, 256, 32};
    // chunk_coordonate world_size = {32, 32, 12};
    // chunk_coordonate world_size = {128, 128, 16};
    // chunk_coordonate world_size = {64, 64, 16};
    world.init(world_size.x, world_size.y, world_size.z);

    for(Uint8 i = 0; i < CHUNK_SIZE; i++)
    for(Uint8 j = 0; j < CHUNK_SIZE; j++)
    for(Uint8 k = 0; k < CHUNK_SIZE; k++)
    {
        for(int z = 0; z <= world.max_chunk_coord.z; z++)
        // for(int z = 0; z <= 0; z++)
        for(int y = 0; y <= world.max_chunk_coord.y; y++)
        for(int x = 0; x <= world.max_chunk_coord.x; x++)
        {
            if(i >= 4 && i <= CHUNK_SIZE-2)
                if(j >= 4 && j <= CHUNK_SIZE-2)
                    if(k >= 4 && k <= CHUNK_SIZE-2)
                        world.chunks[x][y][z].blocks[i][j][k].id = x*16 + y + 1;

            // int wx = x*CHUNK_SIZE+i;
            // int wy = y*CHUNK_SIZE+j;
            // int wz = z*CHUNK_SIZE+k;
            // int wz2 =  world.max_chunk_coord.z*CHUNK_SIZE/2 - wz;
            // wz2 += CHUNK_SIZE;

            // if(wz < world.max_chunk_coord.z*CHUNK_SIZE/2)
            // {
            //     if(wx < 4 || wy < 4 || z == 0 || wz2 > wx/5 || wz2 > wy/5)
            //         world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_SAND;
            // }

            // if(wz < 42 && !world.chunks[x][y][z].blocks[i][j][k].id)
            //     world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_WATER;

            // if(wz < 42 && (wx == world.max_block_coord.x-1 || wy == world.max_block_coord.y-1))
            //     world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_DEBUG;
            
            // if(wx == 150)
            //     world.chunks[x][y][z].blocks[i][j][k].id = 244+((wx+wy+wz)%4);

            // if(wz == 0)
            //     world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_GREEN;
        }
    }
    //////////////////////////////
    world.compress_all_chunks();

    RE.projection_grid.init_pos(world_size.x*CHUNK_SIZE, world_size.y*CHUNK_SIZE, world_size.z*CHUNK_SIZE);
}

void Game::init(GPU_Target* _screen)
{
    std::cout << "\n ##### ISO VOX #####\n";

    state = STATE_MAIN_MENU;
    Current_world_name = "/noworld";

    std::string tmpname = "player";
    player.init_sprite(&GameEvent, &world, &RE, tmpname);

    Show_HUD = true;
    UI.generate_tiles(-1, _screen->w, _screen->h);
    UI.generate_tiles(STATE_CONSTRUCTION, _screen->w, _screen->h);
    UI.generate_tiles(STATE_WORLD_SELECTION, _screen->w, _screen->h);
    UI.generate_tiles(STATE_MAIN_MENU, _screen->w, _screen->h);

    for(int i = TEXTURE_MIN_ID; i < TEXTURE_MAX_NUMBER; i++)
        RE.Textures[i] = std::make_shared<Texture>(i);


    generate_debug_world();

    init_Render_Engine(_screen);
    

    while(!unlocked_blocks.empty())
        unlocked_blocks.pop_back();

    unlocked_blocks.push_front(229);
    unlocked_blocks.push_front(228);
    unlocked_blocks.push_front(227);
    unlocked_blocks.push_front(226);
    unlocked_blocks.push_front(39);
    unlocked_blocks.push_front(38);
    unlocked_blocks.push_front(37);
    unlocked_blocks.push_front(36);
    unlocked_blocks.push_front(35);
    unlocked_blocks.push_front(34);
    unlocked_blocks.push_front(33);
    unlocked_blocks.push_front(32);
    unlocked_blocks.push_front(31);
    unlocked_blocks.push_front(BLOCK_WATER+7);
    unlocked_blocks.push_front(BLOCK_WATER+6);
    unlocked_blocks.push_front(BLOCK_WATER+5);
    unlocked_blocks.push_front(BLOCK_WATER+4);
    unlocked_blocks.push_front(BLOCK_WATER+3);
    unlocked_blocks.push_front(BLOCK_WATER+2);
    unlocked_blocks.push_front(BLOCK_WATER+1);
    unlocked_blocks.push_front(BLOCK_WATER);
    unlocked_blocks.push_front(BLOCK_SAND+6);
    unlocked_blocks.push_front(BLOCK_SAND+5);
    unlocked_blocks.push_front(BLOCK_SAND+4);
    unlocked_blocks.push_front(BLOCK_SAND+3);
    unlocked_blocks.push_front(BLOCK_SAND+2);
    unlocked_blocks.push_front(BLOCK_SAND+1);
    unlocked_blocks.push_front(BLOCK_SAND);
    unlocked_blocks.push_front(1);
    unlocked_blocks.push_front(2);
    unlocked_blocks.push_front(3);
    unlocked_blocks.push_front(4);
    unlocked_blocks.push_front(5);

    bs_max_line = 0;

    auto Current_block = unlocked_blocks.begin();

    for(int i = 0; i < 8; i++)
    {
        Current_block = circularNext(unlocked_blocks, Current_block);
        currentblocks[i] = *Current_block;
        UI.set_ui_current_blocks(i, currentblocks[i]);
    }

    cb_id = 0;

    init_meteos();

    // unlocked_meteo.push_front(METEO_MAIN_MENU);
    unlocked_meteo.push_front(METEO_ANIMATED_SKY);
    unlocked_meteo.push_front(METEO_AZUR_AURORA);
    // unlocked_meteo.push_front(METEO_JADE_AURORA);
    // unlocked_meteo.push_front(METEO_SCARLET_AURORA);
    // unlocked_meteo.push_front(METEO_ORCHID_AURORA);
    unlocked_meteo.push_front(METEO_NEBULA);
    Current_meteo = unlocked_meteo.begin();

    RE.set_global_illumination(meteos[METEO_MAIN_MENU].global_illumination);
    RE.background_shader = meteos[METEO_MAIN_MENU].background_shader;

    // UI.set_ui_current_blocks(*circularPrev(unlocked_blocks, Current_block), *Current_block, *circularNext(unlocked_blocks, Current_block));
    // UI.set_ui_current_blocks(unlocked_blocks, Current_block);


    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

    RE.projection_grid.refresh_all_identical_line();

    float basic_gi[4] = {1, 1, 1, 0.60};
    // float basic_gi[4] = {0.25, 0.25, 0.35, 0.60};
    RE.set_global_illumination(basic_gi);

    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){-50000, -50000});

    world.world_view_position = 0;
}

int Game::load_world_extras(std::string filename, world_extras* extras) {
    std::string total_filename = "saves/";
    total_filename.append(filename);
    total_filename.append("/extras.bin");

    FILE* file = fopen(total_filename.c_str(), "rb");

    if(!file) {
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    fread(extras, sizeof(world_extras), 1, file);

    fclose(file);

    return SAVE_ERROR_NONE;
}

int Game::save_world_extras(std::string filename, world_extras& extras)
{
    std::string total_filename = "saves/";
    total_filename.append(filename);
    total_filename.append("/extras.bin");

    FILE* file = fopen(total_filename.c_str(), "wb");

    if(!file) {
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    fwrite(&extras, sizeof(world_extras), 1, file);

    fclose(file);

    return SAVE_ERROR_NONE;
}

void Game::world_extras_apply(world_extras& extras, world_extras_select extras_select)
{
    if (extras_select.camera_pos)
        RE.target = extras.camera_pos; // peut être il faut d'autres trucs jsp

    if (RE.window.scale != extras.scale && extras_select.scale) {
        GameEvent.add_event(GAME_EVENT_NEWSCALE, extras.scale);
    }

    if (world.world_view_position != extras.world_view_position && extras_select.world_view_position) {
        world.world_view_position = extras.world_view_position;
    }

    if (extras_select.meteo)
    {
        RE.set_global_illumination(meteos[extras.meteoid].global_illumination);
        RE.background_shader = meteos[extras.meteoid].background_shader;
        RE.background_shader_data = meteos[extras.meteoid].add_data;
    }
}

void Game::world_extras_fill(world_extras& extras)
{
    extras.camera_pos = RE.target;
    extras.scale = RE.window.scale;
    extras.world_view_position = world.world_view_position;
    extras.meteoid = *Current_meteo;
}

int Game::load_world(std::string filename, 
               bool new_size, 
               bool recenter_camera, 
               world_extras* extras, 
               world_extras_select extras_select)
{
    // Uint64 start = Get_time_ms();
    // Uint64 end;

    std::string total_filename = "saves/";

    total_filename.append(filename);

    total_filename.append("/world.isosave");

    int status = world.load_from_file(total_filename.c_str());

    if(status == 0) 
    {
        if (extras != nullptr) {
            load_world_extras(filename, extras);
            if (extras_select) {
                world_extras_apply(*extras, extras_select);
            }
        }
        if (extras_select) {
            world_extras we;
            load_world_extras(filename, &we);
            world_extras_apply(we, extras_select);
        }

        if(new_size)
        {
            RE.projection_grid.free_pos();
            RE.projection_grid.init_pos(world.max_block_coord.x, world.max_block_coord.y, world.max_block_coord.z);
        }

        // std::cout << "world load successfully :)\n";

        // RE.world = world;
        // world.compress_all_chunks();

        RE.max_height_render = world.max_block_coord.z;

        RE.set_global_illumination_direction();

        if(recenter_camera)
        {
            RE.center_camera();
            RE.window.scale = DEFAULT_SCALE;
            RE.refresh_sprite_size();
        }

        refresh_world_render();
    }
    else
        std::cout << "world load failed ._. !\n";


    // end = Get_time_ms();

    // std::cout << "Loaded " 
    // << world.max_chunk_coord.x << "x" 
    // << world.max_chunk_coord.y << "x" 
    // << world.max_chunk_coord.z << " world in " 
    // << end-start << "ms\n";

    return status;
}

void Game::create_new_world(coord3D size, std::string &name)
{
    World new_world;

    new_world.init(size.x, size.y, size.z);

    int id;

    for(int x = 0; x <= new_world.max_chunk_coord.x; x++)
    for(int y = 0; y <= new_world.max_chunk_coord.y; y++)
    // for(int z = 0; z <= new_world.max_chunk_coord.z; z++)
    {
        if((x+y)%2)
            id = 31;
        else 
            id = 32;

        memset(new_world.chunks[x][y][0].blocks, id, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(block));
    }

    new_world.compress_all_chunks();

    std::string total_filename = "saves/";
    total_filename.append(name);
    total_filename.append("/world.isosave");

    int status = new_world.save_to_file(total_filename);

    if(status == 0)
    {
        world_extras we;
        world_extras_fill(we);
        save_world_extras(name, we);
        std::cout << "world saved at " << total_filename << "\n";
    }
    else
    {
        std::cout << status << " : failded to save world :(\n";
    }
}

void Game::refresh_world_render()
{
    GameEvent.drop_all_nfs_event();

    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
    RE.projection_grid.clear();
    RE.projection_grid.save_curr_interval();
    RE.refresh_pg_onscreen();
    RE.refresh_pg_block_visible();
    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
}

void Game::refresh_world_render_fast()
{
    GameEvent.drop_all_nfs_event();
    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

    // RE.projection_grid.clear();
    RE.projection_grid.save_curr_interval();
    // RE.refresh_pg_onscreen();
    // RE.refresh_pg_block_visible();
    GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);   
}

void Game::input()
{
    switch (state)
    {
    case STATE_MAIN_MENU :
        input_main_menu();
        break;

    case STATE_WORLD_SELECTION :
        input_world_selection();
        break;
    
    case STATE_BLOCK_SELECTION :
        input_block_selection();
        break;
    
    case STATE_CONSTRUCTION :
        input_construction();
        break;

    case STATE_ADVENTURE :
        input_adventure();
        break;

    default:
        break;
    }

}

void Game::input_main_menu()
{
    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
    SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);

    // std::cout << "\n" << New_world_name;

    while(SDL_PollEvent(&event))
    {
    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {

                    case SDLK_F4 : 

                        if(km & KMOD_LALT)
                        {
                            state = STATE_QUIT;
                        }

                    case SDLK_ESCAPE :
                        state = STATE_QUIT;
                        break;

                    case SDLK_RETURN :
                        if(km & KMOD_LALT)
                            GPU_SetFullscreen(!GPU_GetFullscreen(), false);
                        else
                            state = STATE_WORLD_SELECTION;
                        break;

                    default : break;
                }
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(Menu_hl_name == IDMENU_QUIT)
                {
                    state = STATE_QUIT;
                }
                
                else if(Menu_hl_name == IDMENU_PLAY)
                {
                    state = STATE_WORLD_SELECTION;
                }
            }
            break;

        default : break;
    }
    }
}

void Game::input_world_selection()
{
    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
    SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);

    // std::cout << "\n" << New_world_name;

    while(SDL_PollEvent(&event))
    {
    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {

                    case SDLK_F4 : 

                        if(km & KMOD_LALT)
                        {
                            state = STATE_QUIT;
                        }

                    case SDLK_ESCAPE :
                    {
                        // SDL_ShowCursor(SDL_ENABLE);

                        UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                        state = STATE_MAIN_MENU;

                        // RE.set_global_illumination(meteos[METEO_MAIN_MENU].global_illumination);
                        // RE.background_shader = meteos[METEO_MAIN_MENU].background_shader;
                    }
                        break;

                    case SDLK_RETURN :
                        if(km & KMOD_LALT)
                            GPU_SetFullscreen(!GPU_GetFullscreen(), false);
                        else
                        {
                            // SDL_ShowCursor(SDL_DISABLE);
                            UI.set_ui_hl_mode(RE.highlight_mode);
                            state = STATE_CONSTRUCTION;
                        }
                        break;

                    default : break;
                }
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(New_world_name[0] != '/')
                {
                    // SDL_ShowCursor(SDL_DISABLE);
                    // UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                    UI.set_ui_hl_mode(RE.highlight_mode);
                    state = STATE_CONSTRUCTION;
                }
            }
            break;
        
        case SDL_MOUSEMOTION :
            // New_world_name
            // Current_world_name
            
            // std::cout << New_world_name << '\n';

            if(New_world_name != Current_world_name && New_world_name[0] != '/')
            {
                // && New_world_name != "/noworld"
                if(!GameEvent.is_NFS_reading_to_wpg)
                {

                    RE.highlight_mode = HIGHLIGHT_MOD_NONE;
                    RE.highlight_type = HIGHLIGHT_BLOCKS;
                    UI.set_ui_hl_type(RE.highlight_type);
                    UI.set_ui_hl_mode(RE.highlight_mode);

                    RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

                    // world_extras we;
                    // load_world(New_world_name, true, true, &we, false);

                    world_extras_select wes(false);
                    wes.world_view_position = true;
                    wes.meteo = true;
                    load_world(New_world_name, true, true, nullptr, wes);

                    Current_world_name = New_world_name;
                }
            }
            // Current_world_name = "/noworld";

            break;

        default : break;
    }
    }
}

void Game::input_block_selection()
{
    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
    SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);

    // std::cout << "\n" << New_world_name;

    while(SDL_PollEvent(&event))
    {
    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {
                    case SDLK_F4 : 

                        if(km & KMOD_LALT)
                        {
                            state = STATE_QUIT;
                        }

                    case SDLK_w :
                    case SDLK_ESCAPE :
                        state = STATE_CONSTRUCTION;
                        // SDL_ShowCursor(SDL_DISABLE);
                        UI.set_ui_hl_mode(RE.highlight_mode);
                        break;
                    

                    case SDLK_RETURN :
                        if(km & KMOD_LALT)
                            GPU_SetFullscreen(!GPU_GetFullscreen(), false);
                        break;

                    case SDLK_a :
                    case SDLK_LEFT :
                        cb_id = (cb_id+1)%8;
                        break;
                    
                    case SDLK_RIGHT :
                    case SDLK_e : 
                        cb_id = cb_id-1 < 0 ? 7 : cb_id-1;
                    default : break;
                }
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(new_current_block)
                {
                    currentblocks[cb_id] = new_current_block;
                    UI.set_ui_current_blocks(cb_id, new_current_block);
                }
            }
            break;

        case SDL_MOUSEWHEEL :
            if(km & KMOD_RSHIFT)
            {
                UI.UI_scale += event.wheel.y*0.25;
                if(UI.UI_scale < 0.0)
                    UI.UI_scale = 0.0;
                
                if(UI.UI_scale > 5.0)
                    UI.UI_scale = 5.0;
            }
            else
            {
                bs_max_line -= event.wheel.y;
                int line = (int)(floor(14*UI.UI_scale));

                if(line < 1)
                    line = 1;
                    
                set_in_interval(bs_max_line, 0, unlocked_blocks.size()/line);
            }


            break;

        default : break;
    }
    }
}

void Game::input_construction()
{
    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
    SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);
    int status;

    bool rwr = false;

    while(SDL_PollEvent(&event))
    {
        switch(event.key.type)
        {
            case SDL_KEYDOWN :
                if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE :
                            if(RE.highlight_wcoord2.x >= 0)
                            {
                                RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                                RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            }
                            else
                            {
                                // std::string total_filename = "saves/";
                                // total_filename.append(Current_world_name);
                                // total_filename.append("/world.isosave");

                                // status = world.save_to_file(total_filename);

                                // if(status == 0)
                                // {
                                //     world_extras we;
                                //     world_extras_fill(we);
                                //     save_world_extras(Current_world_name, we);
                                //     std::cout << "world saved !\n";
                                // }
                                // else
                                // {
                                //     std::cout << status << " : failded to save world :(\n";
                                // }

                                state = STATE_WORLD_SELECTION;
                                // SDL_ShowCursor(SDL_ENABLE);
                                UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                            }
                            break;

                        case SDLK_RETURN :
                            if(km & KMOD_LALT)
                                GPU_SetFullscreen(!GPU_GetFullscreen(), false);
                            
                            break;

                        case SDLK_F1 : 
                            RE.shader_features ^= SFEATURE_GLOBAL_ILLUMINATION;
                            break;

                        case SDLK_F2 : 
                            RE.shader_features ^= SFEATURE_AMBIANT_OCCLUSION;
                            break;

                        case SDLK_F3 : 
                            RE.shader_features ^= SFEATURE_BLOCK_BORDERS;
                            break;

                        case SDLK_F4 : 

                            if(km & KMOD_LALT)
                            {
                                state = STATE_QUIT;
                            }
                            RE.shader_features ^= SFEATURE_SHADOWS;
                            break;

                        case SDLK_g :
                            RE.shader_features ^= SFEATURE_GRID;
                            break;

                        case SDLK_F6 : 
                            RE.shader_features ^= SFEATURE_BLOOM;
                            break;

                        case SDLK_TAB :
                            RE.highlight_type = (RE.highlight_type+1)%5;
                            RE.highlight_type = RE.highlight_type == 0 ? 1 : RE.highlight_type;

                            RE.height_volume_tool = -1;

                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

                            UI.set_ui_hl_type(RE.highlight_type);
                            break;

                        case SDLK_0 :
                            UI.set_ui_hl_type(HIGHLIGHT_MOD_NONE);
                            RE.highlight_type  = HIGHLIGHT_MOD_NONE;
                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            break;

                        case SDLK_1 :
                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            UI.set_ui_hl_type(HIGHLIGHT_BLOCKS);
                            RE.highlight_type = HIGHLIGHT_BLOCKS;
                            break;

                        case SDLK_2 :
                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            UI.set_ui_hl_type(HIGHLIGHT_FLOOR);
                            RE.highlight_type = HIGHLIGHT_FLOOR;
                            break;
                        
                        case SDLK_3 :
                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            RE.height_volume_tool = -1;
                            UI.set_ui_hl_type(HIGHLIGHT_WALL);
                            RE.highlight_type = HIGHLIGHT_WALL;
                            break;

                        case SDLK_4 :
                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            RE.height_volume_tool = -1;
                            UI.set_ui_hl_type(HIGHLIGHT_VOLUME);
                            RE.highlight_type = HIGHLIGHT_VOLUME;
                            break;

                        case SDLK_z :
                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            RE.highlight_mode = (RE.highlight_mode+1)%5;
                            UI.set_ui_hl_mode(RE.highlight_mode);
                            break;

                        case SDLK_s :
                            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                            RE.highlight_mode = RE.highlight_mode-1;
                            RE.highlight_mode = RE.highlight_mode < 0 ? 4 : RE.highlight_mode;
                            UI.set_ui_hl_mode(RE.highlight_mode);
                            break;

                        case SDLK_SPACE :
                            RE.center_camera();
                            GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){0, 0});
                            break;
                        
                        case SDLK_d :
                        {
                            RE.rotate_camera(1);

                            GameEvent.drop_all_nfs_event();

                            RE.projection_grid.clear();
                            RE.projection_grid.save_curr_interval();
                            // GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
                            RE.refresh_pg_onscreen();
                            RE.refresh_pg_block_visible();
                            GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                            GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                        }
                            break;
                        
                        case SDLK_q :
                        {
                            RE.rotate_camera(-1);

                            GameEvent.drop_all_nfs_event();

                            RE.projection_grid.clear();
                            RE.projection_grid.save_curr_interval();
                            // GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
                            RE.refresh_pg_onscreen();
                            RE.refresh_pg_block_visible();
                            GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                            GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                        }
                            break;

                        case SDLK_PAGEUP :
                            if(RE.max_height_render != world.max_block_coord.z)
                            {
                                RE.max_height_render += CHUNK_SIZE;
                                if(RE.max_height_render > world.max_block_coord.z)
                                    RE.max_height_render = world.max_block_coord.z;

                                refresh_world_render();
                            }
                            break;

                        case SDLK_PAGEDOWN :
                            if(RE.max_height_render > 1)
                            {
                                RE.max_height_render -= CHUNK_SIZE;
                                if(RE.max_height_render < 1)
                                    RE.max_height_render = 1;

                                refresh_world_render();
                            }
                            break;

                        case SDLK_F5:
                        {
                            std::string total_filename = "saves/";
                            total_filename.append(Current_world_name);
                            total_filename.append("/world.isosave");

                            status = world.save_to_file(total_filename);

                            if(status == 0)
                            {
                                world_extras we;
                                world_extras_fill(we);
                                save_world_extras(Current_world_name, we);
                                std::cout << "world saved at " << total_filename << "\n";
                            }
                            else
                            {
                                std::cout << status << " : failded to save world :(\n";
                            }
                        }
                            break;

                        case SDLK_F9 :                            
                            GameEvent.drop_game_event();
                            if(!GameEvent.is_NFS_reading_to_wpg)
                                load_world(Current_world_name);
                        break;

                        case SDLK_r :
                            RE.projection_grid.save_curr_interval();
                            RE.refresh_pg_onscreen();
                            break;

                        case SDLK_LEFT :
                        case SDLK_a :
                            cb_id = (cb_id+1)%8;
                            break;
                        
                        case SDLK_RIGHT :
                        case SDLK_e : 

                            cb_id = cb_id-1 < 0 ? 7 : cb_id-1;
                            break;
                        
                        case SDLK_UP :
                            Current_meteo = circularNext(unlocked_meteo, Current_meteo);
                            RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
                            RE.background_shader = meteos[*Current_meteo].background_shader;
                            RE.background_shader_data = meteos[*Current_meteo].add_data;
                            break;

                        case SDLK_DOWN :
                            Current_meteo = circularPrev(unlocked_meteo, Current_meteo);
                            RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
                            RE.background_shader = meteos[*Current_meteo].background_shader;
                            RE.background_shader_data = meteos[*Current_meteo].add_data;
                            break;

                        case SDLK_h :
                            std::cout << "UI_scale :" << UI.UI_scale;
                            std::cout << "sprite counter : " << RE.sprite_counter << '\n';
                            std::cout << "scale : 1/" << 1/RE.window.scale << '\n';
                            std::cout << "block_size : " << RE.block_onscreen_size << '\n';
                            std::cout << "World view position : " << world.world_view_position << '\n';
                            break;
                        
                        case SDLK_p :
                            // std::cout << "\nrefreshing shaders...";
                            system("cls");
                            RE.refresh_shaders();
                            RE.DFIB_shader.activate();
                            GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.DFIB_shader.get_location("ao"), 2);
                            GPU_SetShaderImage(RE.Textures[SHADERTEXT_WATER]->ptr, RE.DFIB_shader.get_location("water"), 4);
                            GPU_SetShaderImage(RE.Textures[BLOCK_BORDER]->ptr, RE.DFIB_shader.get_location("border"), 5);
                            GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.DFIB_shader.get_location("normal"), 6);
                            GPU_SetShaderImage(RE.Textures[BLOCK_LIGHT]->ptr, RE.DFIB_shader.get_location("light"), 7);
                            RE.DFIB_shader.deactivate();
                            
                            init_meteos();
                            RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
                            RE.background_shader = meteos[*Current_meteo].background_shader;
                            RE.background_shader_data = meteos[*Current_meteo].add_data;
                            break;
                        
                        case SDLK_f : 
                            
                            if(RE.highlight_type != HIGHLIGHT_PIPETTE)
                            {
                                Current_HL_type = RE.highlight_type;
                                RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                                RE.highlight_type = HIGHLIGHT_PIPETTE;
                                UI.set_ui_hl_type(HIGHLIGHT_PIPETTE);
                            }
                            else
                            {
                                RE.highlight_type = Current_HL_type;
                                RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                                UI.set_ui_hl_type(RE.highlight_type);
                            }

                            break;

                        case SDLK_x : 
                            Show_HUD = !Show_HUD;
                            break;

                        case SDLK_w :
                            state = STATE_BLOCK_SELECTION;
                            // SDL_ShowCursor(SDL_ENABLE);
                            UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                            break;

                        case SDLK_KP_1 :
                            state = STATE_ADVENTURE;
                            UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                            break;

                        default : break;
                    }
                break;

            case SDL_MOUSEMOTION :
                if(event.motion.state == SDL_BUTTON_RMASK)
                {
                    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){event.motion.xrel, event.motion.yrel});
                }
                break;

            case SDL_MOUSEWHEEL :

                if(km & KMOD_LSHIFT)
                {
                    world.find_highest_nonemptychunk();

                    if(RE.max_height_render > (world.highest_nonemptychunk+1)*CHUNK_SIZE)
                    {
                        RE.max_height_render = (world.highest_nonemptychunk+1)*CHUNK_SIZE + event.wheel.y;
                    }
                    else
                        RE.max_height_render += event.wheel.y;

                    if(RE.max_height_render > world.max_block_coord.z)
                        RE.max_height_render = world.max_block_coord.z;

                    else if(RE.max_height_render < 1)
                        RE.max_height_render = 1;

                    rwr = true;
                }
                else if(km & KMOD_LCTRL)
                {
                    RE.height_volume_tool += event.wheel.y;
                    set_in_interval(RE.height_volume_tool, 0, world.max_block_coord.z);
                }
                else if(km & KMOD_RCTRL)
                {
                    RE.height_volume_tool += 8*event.wheel.y;
                    set_in_interval(RE.height_volume_tool, 0, world.max_block_coord.z);
                }
                else if(km & KMOD_RSHIFT)
                {
                    UI.UI_scale += event.wheel.y*0.25;
                    if(UI.UI_scale < 0.0)
                        UI.UI_scale = 0.0;
                    
                    if(UI.UI_scale > 5.0)
                        UI.UI_scale = 5.0;
                }
                else
                {
                    GameEvent.add_event(GAME_EVENT_ADDSCALE, event.wheel.y);
                }

                break;
            
            case SDL_MOUSEBUTTONDOWN :
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    if(RE.highlight_type == HIGHLIGHT_PIPETTE)
                    {
                        Uint8 id = world.get_block_id_wcoord(RE.highlight_wcoord.x, RE.highlight_wcoord.y, RE.highlight_wcoord.z);

                        auto pipette = std::find(unlocked_blocks.begin(), unlocked_blocks.end(), id);

                        if(pipette != unlocked_blocks.end() || *pipette != *unlocked_blocks.end())
                        {
                            // std::cout << "\npipette moment " << id;

                            UI.set_ui_current_blocks(cb_id, *pipette);
                            currentblocks[cb_id] = *pipette;
                        }

                    }
                    else if(RE.highlight_wcoord2.x == HIGHLIGHT_NOCOORD && RE.highlight_type >= HIGHLIGHT_FLOOR)
                    {
                        RE.highlight_wcoord2 = RE.highlight_wcoord;
                    }
                    else if(RE.highlight_mode >= HIGHLIGHT_MOD_REPLACE)
                    {
                        GameEvent.add_event(GAME_EVENT_PLAYER_BLOCK_MOD, (coord3D)RE.highlight_wcoord, currentblocks[cb_id]);
                    }
                    else if(RE.highlight_mode == HIGHLIGHT_MOD_DELETE)
                    {
                        GameEvent.add_event(GAME_EVENT_PLAYER_BLOCK_MOD, (coord3D)RE.highlight_wcoord, BLOCK_EMPTY);
                    }
                }
                else
                if(event.button.button == SDL_BUTTON_MIDDLE)
                {
                    RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                }
                break;

            default:
                break;
            }
    }

    if(rwr)
        refresh_world_render();
};

void Game::input_adventure()
{
    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
    SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);

    while(SDL_PollEvent(&event))
    {
    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {

                    case SDLK_F4 : 
                        if(km & KMOD_LALT)
                        {
                            state = STATE_QUIT;
                        }

                    case SDLK_ESCAPE :
                        state = STATE_CONSTRUCTION;
                        UI.set_ui_hl_mode(RE.highlight_mode);
                        break;

                    case SDLK_RETURN :
                        if(km & KMOD_LALT)
                            GPU_SetFullscreen(!GPU_GetFullscreen(), false);
                        else
                        {
                            std::cout << "spawning player sprite\n";

                            fcoord3D initial_playerpos = {world.max_block_coord.x*0.5f, 
                                                          world.max_block_coord.y*0.5f,
                                                          2.0};

                            player.tp(initial_playerpos);

                            // player.tp({50, 50, 2});

                            next_frame_render_refresh = true;
                        }
                        break;
                    
                    case SDLK_DELETE :

                        std::cout << "removing player sprite from world\n";
                        player.remove();
                        next_frame_render_refresh = true;

                        break;
                    
                    case SDLK_r :
                        RE.projection_grid.save_curr_interval();
                        RE.refresh_pg_onscreen();
                        break;

                    default : break;
                }
            break;
        
        case SDL_MOUSEMOTION :
            if(event.motion.state == SDL_BUTTON_RMASK)
            {
                GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){event.motion.xrel, event.motion.yrel});
            }
            break;

        case SDL_MOUSEWHEEL :
            
            GameEvent.add_event(GAME_EVENT_ADDSCALE, event.wheel.y);

            break;

        default:
            break;
        }
    }

    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    fcoord3D player_vel = {0.0, 0.0, 0.0};
    // fcoord3D player_vel = {0.05*cos(timems*0.005), 0.0, 0.0};

    float speed = 1.0;

    if(keystate[ SDL_SCANCODE_W ])
    {
        player_vel.x += 1.0*speed;
        player_vel.y += 1.0*speed;
    }
    if(keystate[ SDL_SCANCODE_S ])
    {
        player_vel.x -= 1.0*speed;
        player_vel.y -= 1.0*speed;
    }
    if(keystate[ SDL_SCANCODE_A ])
    {
        player_vel.x += 1.0*speed;
        player_vel.y -= 1.0*speed;
    }
    if(keystate[ SDL_SCANCODE_D ])
    {
        player_vel.x -= 1.0*speed;
        player_vel.y += 1.0*speed;
    }

    if(player.move(player_vel))
    {
        next_frame_render_refresh = true;
    }
}

int Game::mainloop()
{
    while(state)
    {
        timems = Get_time_ms() - timems_start;
        
        input();
        GameEvent.handle();
        RE.render_frame();

        if(Show_HUD || state != STATE_CONSTRUCTION)
            UI.render_frame(state, RE.screen, 
                            cb_id, bs_max_line, new_current_block, unlocked_blocks, 
                            New_world_name, Menu_hl_name);

        GPU_Flip(RE.screen);
    }

    GameEvent.game_is_running = false;
    GameEvent.handle();

    SDL_WaitThread(RE.SecondaryThread, NULL);

    GameEvent.drop_all_nfs_event();
    SDL_CondSignal(GameEvent.new_nfs_event);
    SDL_WaitThread(GameEvent.NFS_Thread, NULL);

    return state;
}