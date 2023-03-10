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


Game::Game(GPU_Target* _screen) :  RE(world), GameEvent(RE), physics_engine(&world, &GameEvent)
{
    world.SetPhysicsEngine(&physics_engine);
    GameEvent.SetPhysicsEngine(&physics_engine); 
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

    RE.final_world_render = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.final_world_render, 0, 0);
    RE.screen2 = GPU_LoadTarget(RE.final_world_render);

    RE.opaque_world_render = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.opaque_world_render, 0, 0);
    RE.screen3 = GPU_LoadTarget(RE.opaque_world_render);

    RE.background_image = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.background_image, 0, 0);
    RE.background = GPU_LoadTarget(RE.background_image);

    RE.depth_fbo_image = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.depth_fbo_image, 0, 0);
    RE.depth_fbo = GPU_LoadTarget(RE.depth_fbo_image);

    UI.UI_image = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(UI.UI_image, 0, 0);
    UI.UI = GPU_LoadTarget(UI.UI_image);

    RE.highlight_mode = HIGHLIGHT_MOD_NONE;
    RE.highlight_type = HIGHLIGHT_BLOCKS;

    RE.shader.activate();
    GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.shader.get_location("ao"), 2);
    GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.shader.get_location("water"), 4);
    GPU_SetShaderImage(RE.Textures[BLOCK_BORDER]->ptr, RE.shader.get_location("border"), 5);
    GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.shader.get_location("normal"), 6);
    GPU_SetShaderImage(RE.Textures[BLOCK_PARTS]->ptr, RE.shader.get_location("parts"), 7);
    RE.shader.deactivate();

    RE.max_height_render = world.max_block_coord.z;

    GPU_AddDepthBuffer(RE.screen2);
    GPU_SetDepthFunction(RE.screen2, GPU_LEQUAL);

    GPU_AddDepthBuffer(RE.depth_fbo);
    GPU_SetDepthFunction(RE.depth_fbo, GPU_ALWAYS);
}

void Game::generate_debug_world()
{
    // chunk_coordonate world_size = {16, 16, 16}; // hut
    chunk_coordonate world_size = {32, 32, 32}; // plain
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
            int wx = x*CHUNK_SIZE+i;
            int wy = y*CHUNK_SIZE+j;
            int wz = z*CHUNK_SIZE+k;
            int wz2 =  world.max_chunk_coord.z*CHUNK_SIZE/2 - wz;
            wz2 += CHUNK_SIZE;

            if(wz < world.max_chunk_coord.z*CHUNK_SIZE/2)
            {
                if(wx < 4 || wy < 4 || z == 0 || wz2 > wx/5 || wz2 > wy/5)
                    world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_SAND;
            }

            if(wz < 42 && !world.chunks[x][y][z].blocks[i][j][k].id)
                world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_WATER;

            if(wz < 42 && (wx == world.max_block_coord.x-1 || wy == world.max_block_coord.y-1))
                world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_DEBUG;
            
            if(wx == 150)
                world.chunks[x][y][z].blocks[i][j][k].id = 244+((wx+wy+wz)%4);

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

    state = STATE_MENU;

    UI.generate_tiles(STATE_MAIN, _screen->w, _screen->h);
    UI.generate_tiles(STATE_MENU, _screen->w, _screen->h);

    for(int i = TEXTURE_MIN_ID; i < TEXTURE_MAX_NUMBER; i++)
        RE.Textures[i] = std::make_shared<Texture>(i);


    generate_debug_world();

    init_Render_Engine(_screen);
    

    while(!unlocked_blocks.empty())
        unlocked_blocks.pop_back();

    unlocked_blocks.push_front(BLOCK_WATER+4);
    unlocked_blocks.push_front(BLOCK_WATER+3);
    unlocked_blocks.push_front(BLOCK_WATER+2);
    unlocked_blocks.push_front(BLOCK_WATER+1);
    unlocked_blocks.push_front(BLOCK_WATER);
    unlocked_blocks.push_front(BLOCK_DEBUG);
    unlocked_blocks.push_front(BLOCK_BLUE);
    unlocked_blocks.push_front(BLOCK_RED);
    unlocked_blocks.push_front(BLOCK_GREEN);
    unlocked_blocks.push_front(BLOCK_SAND);

    Current_block = unlocked_blocks.begin();

    // UI.set_ui_current_blocks(*circularPrev(unlocked_blocks, Current_block), *Current_block, *circularNext(unlocked_blocks, Current_block));
    UI.set_ui_current_blocks(unlocked_blocks, Current_block);


    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

    RE.projection_grid.refresh_all_identical_line();

    float basic_gi[4] = {1, 1, 1, 0.60};
    // float basic_gi[4] = {0.25, 0.25, 0.35, 0.60};
    RE.set_global_illumination(basic_gi);

    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){RE.window.size.x/2, RE.window.size.y/2});
}

int Game::load_world(std::string filename, bool new_size, bool recenter_camera, bool verbose)
{
    Uint64 start = Get_time_ms();
    Uint64 end_load;
    Uint64 end_refresh;

    physics_engine.clear_events();

    std::string total_filename = "saves/";

    total_filename.append(filename);

    total_filename.append("/world.isosave");


    int status = world.load_from_file(total_filename.c_str());
    end_load = Get_time_ms();

    if(status == 0) 
    {
        if(new_size)
        {
            RE.projection_grid.free_pos();
            RE.projection_grid.init_pos(world.max_block_coord.x, world.max_block_coord.y, world.max_block_coord.z);
        }

        // std::cout << "world load successfully :)\n";

        // RE.world = world;
        // world.compress_all_chunks();

        RE.max_height_render = world.max_block_coord.z;

        if(recenter_camera)
        {
            RE.center_camera();
            RE.window.scale = DEFAULT_SCALE;
            RE.refresh_sprite_size();
        }

        refresh_world_render();

        end_refresh = Get_time_ms();

        if (verbose) {
            std::cout << "Loaded " << world.max_chunk_coord.x << "x" << world.max_chunk_coord.y << "x" << world.max_chunk_coord.z << " world successfully" << std::endl;
            std::cout << "Load time: " << end_load-start << "ms" << std::endl;
            std::cout << "Refresh time: " << end_refresh-end_load << "ms" << std::endl;
            std::cout << "Total time: " << end_refresh-start << "ms" << std::endl;
        }

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

void Game::refresh_world_render()
{
    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

    GameEvent.drop_all_nfs_event();
    RE.projection_grid.clear();
    RE.projection_grid.save_curr_interval();
    RE.refresh_pg_onscreen();
    RE.refresh_pg_block_visible();
    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
}

void Game::refresh_world_render_fast()
{
    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

    GameEvent.drop_all_nfs_event();
    RE.projection_grid.clear();
    RE.projection_grid.save_curr_interval();
    // RE.refresh_pg_onscreen();
    // RE.refresh_pg_block_visible();
    GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);   
}

void Game::input()
{
    if(state == STATE_MAIN)
        input_maingame();
    
    else if(state == STATE_MENU)
        input_mainmenu();
}

void Game::input_mainmenu()
{
    SDL_Event event;
    // SDL_Keymod km = SDL_GetModState();
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
                    case SDLK_ESCAPE :
                        state = STATE_QUIT;
                        break;

                    case SDLK_KP_ENTER :
                        state = STATE_MAIN;
                        break;

                    default : break;
                }
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                state = STATE_MAIN;
            }
            break;
        
        case SDL_MOUSEMOTION :
            // New_world_name
            // Current_world_name

            if(New_world_name != Current_world_name)
            {
                if(!GameEvent.is_NFS_reading_to_wpg)
                {
                    RE.highlight_mode = HIGHLIGHT_MOD_NONE;
                    RE.highlight_type = HIGHLIGHT_BLOCKS;

                    RE.highlight_wcoord = {-1, -1, -1};
                    RE.highlight_wcoord2 = {-1, -1, -1};

                    load_world(New_world_name, true, true);
                    Current_world_name = New_world_name;
                }
            }

            break;

        default : break;
    }
    }
}

void Game::input_maingame()
{
    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
    SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);
    int status;

    while(SDL_PollEvent(&event))
    {
        switch(event.key.type)
        {
            case SDL_KEYDOWN :
                if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE :
                            state = STATE_MENU;
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
                            RE.shader_features ^= SFEATURE_SHADOWS;
                            break;

                        case SDLK_F5 : 
                            RE.shader_features ^= SFEATURE_BLOOM;
                            break;

                        case SDLK_0 :
                            UI.set_ui_hl_type(HIGHLIGHT_MOD_NONE);
                            RE.highlight_mode  = HIGHLIGHT_MOD_NONE;
                            RE.highlight_wcoord = {-1, -1, -1};
                            break;

                        case SDLK_1 :
                            UI.set_ui_hl_type(HIGHLIGHT_BLOCKS);
                            RE.highlight_type = HIGHLIGHT_BLOCKS;
                            break;

                        case SDLK_2 :
                            UI.set_ui_hl_type(HIGHLIGHT_FLOOR);
                            RE.highlight_type = HIGHLIGHT_FLOOR;
                            break;
                        
                        case SDLK_3 :
                            UI.set_ui_hl_type(HIGHLIGHT_WALL_X);
                            RE.highlight_type = HIGHLIGHT_WALL_X;
                            break;

                        case SDLK_4 :
                            UI.set_ui_hl_type(HIGHLIGHT_WALL_Y);
                            RE.highlight_type = HIGHLIGHT_WALL_Y;
                            break;

                        case SDLK_5 :
                            UI.set_ui_hl_type(HIGHLIGHT_VOLUME);
                            RE.highlight_type = HIGHLIGHT_VOLUME;
                            break;

                        case SDLK_a :
                            RE.highlight_mode = (RE.highlight_mode+1)%5;
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

                        case SDLK_F6:
                        {
                            Uint64 start = Get_time_ms();
                            std::string total_filename = "saves/";
                            total_filename.append(Current_world_name);
                            // total_filename.append("plain");
                            total_filename.append("/world.isosave");

                            status = world.save_to_file(total_filename);

                            Uint64 end = Get_time_ms();
                            if(status == 0)
                            {
                                std::cout << "world saved !\n";
                                std::cout << "time: " << end - start << " ms\n";
                            }
                            else
                            {
                                std::cout << status << " : failded to save world :(\n";
                            }
                            break;
                        }

                        case SDLK_F7: {
                            load_world(Current_world_name, true, false, true);
                            break;
                        }

                        case SDLK_r :
                            RE.projection_grid.save_curr_interval();
                            RE.refresh_pg_onscreen();
                            break;

                        case SDLK_LEFT :
                            Current_block = circularNext(unlocked_blocks, Current_block);
                            // UI.set_ui_current_blocks(*circularPrev(unlocked_blocks, Current_block), *Current_block, *circularNext(unlocked_blocks, Current_block));
                            UI.set_ui_current_blocks(unlocked_blocks, Current_block);
                            break;
                        
                        case SDLK_RIGHT :
                            Current_block = circularPrev(unlocked_blocks, Current_block);
                            // UI.set_ui_current_blocks(*circularPrev(unlocked_blocks, Current_block), *Current_block, *circularNext(unlocked_blocks, Current_block));
                            UI.set_ui_current_blocks(unlocked_blocks, Current_block);
                            break;

                        case SDLK_TAB :
                            std::cout << "sprite counter : " << RE.sprite_counter << '\n';
                            std::cout << "scale : 1/" << 1/RE.window.scale << '\n';
                            std::cout << "block_size : " << RE.block_onscreen_size << '\n';
                            std::cout << "World view position : " << world.world_view_position << '\n';
                            break;
                        
                        case SDLK_KP_MINUS :
                            std::cout << "toggle physics engine\n";
                            physics_engine.toggle_running();
                            break;
                        
                        case SDLK_KP_PERIOD :
                            std::cout << "Physics engine step\n";
                            physics_engine.tick();
                            break;
                        
                        // case SDLK_KP_ENTER : 
                        //     for(int i = 0; i < 500; i++)
                        //     {
                        //         block_coordonate test = {0};
                        //         test.chunk.z = 15;
                        //         test.chunk.x = 7;
                        //         test.chunk.y = 7;
                        //         GameEvent.add_event(GAME_EVENT_SINGLE_BLOCK_MOD_ALT, test, BLOCK_RED);
                        //     }
                        //     break;

                        case SDLK_w: { 
                            if (RE.highlight_mode > HIGHLIGHT_MOD_NONE) {
                                if (RE.highlight_wcoord2.x == -1 && RE.highlight_wcoord2.y == -1 && RE.highlight_wcoord2.z == -1) {
                                    std::cout << "Selection Pos x = " << RE.highlight_wcoord.x << ", y = " << RE.highlight_wcoord.y << ", z = " << RE.highlight_wcoord.z << std::endl;
                                }
                                else {
                                std::cout << "Selection begin: x = " << RE.highlight_wcoord2.x 
                                << ", y = " << RE.highlight_wcoord2.y 
                                << ", z = " << RE.highlight_wcoord2.z << std::endl;

                                std::cout << "Selection end: x = " 
                                << RE.highlight_wcoord.x 
                                << ", y = " << RE.highlight_wcoord.y 
                                << ", z = " << RE.highlight_wcoord.z << std::endl;

                                std::cout << "Selection length: x = " 
                                << abs(RE.highlight_wcoord.x - RE.highlight_wcoord2.x) + 1 
                                << ", y = " << abs(RE.highlight_wcoord.y - RE.highlight_wcoord2.y) +1 
                                << ", z = " << abs(RE.highlight_wcoord.z - RE.highlight_wcoord2.z) +1 
                                << std::endl << std::endl;
                                }
                                
                            }
                            break;
                        }

                        default : break;
                    }
                break;

            case SDL_MOUSEMOTION :
                // std::cout << event.motion.state << '\n';
                if(event.motion.state == SDL_BUTTON_RMASK)
                {
                    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){event.motion.xrel, event.motion.yrel});
                }
                break;

            case SDL_MOUSEWHEEL :

                if(km & KMOD_LSHIFT)
                {
                    RE.max_height_render += event.wheel.y;

                    if(RE.max_height_render > world.max_block_coord.z)
                        RE.max_height_render = world.max_block_coord.z;

                    else if(RE.max_height_render < 1)
                        RE.max_height_render = 1;

                    refresh_world_render();
                }
                else
                {
                    GameEvent.add_event(GAME_EVENT_ADDSCALE, event.wheel.y);
                }

                break;
            
            case SDL_MOUSEBUTTONDOWN :
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    // if(RE.highlight_mode == HIGHLIGHT_REMOVE)
                    //     GameEvent.add_event(GAME_EVENT_SINGLE_BLOCK_MOD, (coord3D)RE.highlight_wcoord, BLOCK_EMPTY);
                    
                    // else if(RE.highlight_mode == HIGHLIGHT_PLACE || RE.highlight_mode == HIGHLIGHT_PLACE_ALT)
                    //     GameEvent.add_event(GAME_EVENT_SINGLE_BLOCK_MOD, (coord3D)RE.highlight_wcoord, Current_block);
                    
                    

                    if(RE.highlight_wcoord2.x == -1 && RE.highlight_type >= HIGHLIGHT_FLOOR)
                    {
                        RE.highlight_wcoord2 = RE.highlight_wcoord;
                    }
                    else if(RE.highlight_mode >= HIGHLIGHT_MOD_REPLACE)
                    {
                        GameEvent.add_event(GAME_EVENT_SINGLE_BLOCK_MOD, (coord3D)RE.highlight_wcoord, *Current_block);

                        /*
                        if (*Current_block == BLOCK_WATER)
                        {
                            PhysicsEventWater *new_event = new PhysicsEventWater(&world, &physics_engine, &GameEvent, {RE.highlight_wcoord.x, RE.highlight_wcoord.y, RE.highlight_wcoord.z});
                            std::cout << "new event added at " << RE.highlight_wcoord.x << " " << RE.highlight_wcoord.y << " " << RE.highlight_wcoord.z << "\n";
                            physics_engine.add_event(new_event);
                        }
                        */
                    }
                    else if(RE.highlight_mode == HIGHLIGHT_MOD_DELETE)
                    {
                        GameEvent.add_event(GAME_EVENT_SINGLE_BLOCK_MOD, (coord3D)RE.highlight_wcoord, BLOCK_EMPTY);
                    }
                }
                break;

            

            default:
                break;
            }
    }
};

int Game::mainloop()
{
    while(state)
    {
        input();
        GameEvent.handle();
        RE.render_frame();
        UI.render_frame(state, RE.screen, New_world_name);

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