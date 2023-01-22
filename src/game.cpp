#include <game.hpp>

Game::Game(GPU_Target* _screen) : RE(world), GameEvent(RE), physics_engine()
{
    state = STATE_QUIT;
    init(_screen);
}

void Game::init_Render_Engine(GPU_Target* _screen)
{
    RE.highlight_mode = HIGHLIGHT_NONE;
    RE.screen = _screen;
    RE.window.size.x = DEFAULT_WINDOWS_W;
    RE.window.size.y = DEFAULT_WINDOWS_H;
    RE.GameEvent = &GameEvent;

    RE.window.scale = DEFAULT_SCALE;

    RE.world = world;

    RE.target.x = 0;
    RE.target.y = 0;

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

    RE.shader.activate();
    GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.shader.get_location("normal"), 1);
    GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.shader.get_location("ao"), 2);
    GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.shader.get_location("water"), 4);
    GPU_SetShaderImage(RE.Textures[BLOCK_BORDER]->ptr, RE.shader.get_location("border"), 5);
    RE.shader.deactivate();

    RE.max_height_render = world.max_block_coord.z;

    GPU_AddDepthBuffer(RE.screen2);
    GPU_SetDepthFunction(RE.screen2, GPU_LEQUAL);
}

void Game::generate_debug_world()
{
    // chunk_coordonate world_size = {256, 256, 32};
    chunk_coordonate world_size = {32, 32, 12};
    // chunk_coordonate world_size = {128, 128, 16};
    // chunk_coordonate world_size = {64, 64, 12};
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

            // if(wz < 42 && !world.chunk[x][y][z].block[i][j][k].id)
            //     world.chunk[x][y][z].block[i][j][k].id = BLOCK_WATER;

            if(wz < 42 && (wx == world.max_block_coord.x-1 || wy == world.max_block_coord.y-1))
                world.chunks[x][y][z].blocks[i][j][k].id = BLOCK_DEBUG;
            
            // if(wx == 150 && wz > 43) {
            //     const int glass_size = 8;
            //     const int glass[] = {
            //         BLOCK_GLASS_RED,
            //         BLOCK_GLASS_BLUE,
            //         BLOCK_GLASS_GREEN,
            //         BLOCK_GLASS_YELLOW,
            //         BLOCK_GLASS_CYAN,
            //         BLOCK_GLASS_MAGENTA
            //     };

            //     int color = (wy / glass_size) % (sizeof(glass) / sizeof(glass[0])); 

            //     world.chunks[x][y][z].blocks[i][j][k].id = glass[color];
            // }

            // if(wz == 0 && wx == 15)
            //     world.chunk[x][y][z].block[i][j][k].id = BLOCK_GREEN;

            // if((i+j)%2)
            //     world.chunk[x][y][z].block[i][j][k].id = BLOCK_GREEN;
            // else
            //     world.chunk[x][y][z].block[i][j][k].id = BLOCK_BLUE;
        }
    }
    //////////////////////////////
    world.compress_all_chunks();

    RE.projection_grid.init_pos(world_size.x*CHUNK_SIZE, world_size.y*CHUNK_SIZE, world_size.z*CHUNK_SIZE);
}

void Game::init(GPU_Target* _screen)
{
    state = STATE_MAIN;

    for(int i = TEXTURE_MIN_ID; i < TEXTURE_MAX_NUMBER; i++)
        RE.Textures[i] = std::make_shared<Texture>(i);

    generate_debug_world();

    init_Render_Engine(_screen);
    Current_block = BLOCK_PALETTE[0];
    Current_block_n = 0;

    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

    RE.projection_grid.refresh_all_identical_line();

    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){RE.window.size.x/2, RE.window.size.y/2});
}

int Game::load_world(const char* filename, bool new_size)
{
    Uint64 start = Get_time_ms();
    int status = world.load_from_file(filename);
    Uint64 load_end = Get_time_ms();

    if(status == 0) 
    {
        if(new_size)
        {
            RE.projection_grid.free_pos();
            RE.projection_grid.init_pos(world.max_block_coord.x, world.max_block_coord.y, world.max_block_coord.z);
        }
        
        Uint64 render_end = Get_time_ms();
        std::cout << "world load successfull :)\n";
        std::cout << "load time : " << load_end - start << " ms\n";
        std::cout << "render time : " << render_end - load_end << " ms\n";
        std::cout << "total time : " << render_end - start << " ms\n";

        // RE.world = world;
        // world.compress_all_chunks();

        RE.max_height_render = world.max_block_coord.z;
        refresh_world_render();
    }
    else
        std::cout << "world load failed ._. !\n";

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

void Game::input()
{
    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
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
                            state = STATE_QUIT;
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
                            RE.highlight_mode  = HIGHLIGHT_NONE;
                            RE.highlight_wcoord = {-1, -1, -1};
                            break;

                        case SDLK_1 :
                            RE.highlight_mode = HIGHLIGHT_REMOVE;
                            break;

                        case SDLK_2 :
                            RE.highlight_mode = HIGHLIGHT_PLACE;
                            break;
                        
                        case SDLK_3 :
                            RE.highlight_mode = HIGHLIGHT_PLACE_ALT;
                            break;

                        case SDLK_4 :
                            RE.highlight_mode = HIGHLIGHT_DEBUG;
                            break;

                        case SDLK_SPACE :
                            GameEvent.add_event(GAME_EVENT_NEWSCALE, PANORAMA_SCALE_THRESHOLD);
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

                                RE.projection_grid.save_curr_interval();
                                GameEvent.drop_all_nfs_event();
                                GameEvent.add_nfs_event(NFS_OP_PG_MHR);
                                GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                                GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                            }
                            break;

                        case SDLK_PAGEDOWN :
                            if(RE.max_height_render != 0)
                            {
                                RE.max_height_render -= CHUNK_SIZE;
                                if(RE.max_height_render < 0)
                                    RE.max_height_render = 0;

                                RE.projection_grid.save_curr_interval();
                                GameEvent.drop_all_nfs_event();

                                GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
                                GameEvent.add_nfs_event(NFS_OP_PG_MHR);
                                GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                                GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                            }
                            break;

                        case SDLK_F6:
                        {
                            Uint64 start = Get_time_ms();
                            status = world.save_to_file("test.worldsave");
                            Uint64 end = Get_time_ms();

                            if(status == 0)
                            {
                                std::cout << "world saved !\n";
                                std::cout << "time: " << end - start << " ms\n";
                            }
                            else
                            {
                                std::cout << "failded to save world :(\n";
                            }
                            break;
                        }

                        case SDLK_F7:
                        {
                            load_world("test.worldsave");
                            break;
                        }

                        case SDLK_r :
                            RE.projection_grid.save_curr_interval();
                            RE.refresh_pg_onscreen();
                            break;

                        case SDLK_LEFT :
                            Current_block_n++;
                            if (Current_block_n % sizeof(BLOCK_PALETTE) == 0)
                                Current_block_n = 0;
                            Current_block = BLOCK_PALETTE[Current_block_n];
                            std::cout << "Current block id: " << (int)BLOCK_PALETTE[Current_block_n] << '\n';
                            std::cout << "Current block   : " << Current_block_n << '\n';
                            break;
                        
                        case SDLK_RIGHT :
                            Current_block_n--;
                            if (Current_block_n < 0)
                                Current_block_n = sizeof(BLOCK_PALETTE) - 1;
                            Current_block = BLOCK_PALETTE[Current_block_n];
                            std::cout << "Current block id: " << (int)BLOCK_PALETTE[Current_block_n] << '\n';
                            std::cout << "Current block   : " << Current_block_n << '\n';
                            break;

                        case SDLK_TAB :
                            std::cout << "sprite counter : " << RE.sprite_counter << '\n';
                            std::cout << "scale : 1/" << 1/RE.window.scale << '\n';
                            std::cout << "block_size : " << RE.block_onscreen_size << '\n';
                            std::cout << "World view position : " << world.world_view_position << '\n';
                            break;

                        default :
                            break;
                    }
                break;

            case SDL_MOUSEMOTION :
                if(event.motion.state == SDL_BUTTON_LMASK)
                {
                    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){event.motion.xrel, event.motion.yrel});
                }
                break;

            case SDL_MOUSEWHEEL :
                if(km == 4096)
                {
                    GameEvent.add_event(GAME_EVENT_ADDSCALE, event.wheel.y);

                }
                else if(km & KMOD_LSHIFT)
                {
                    RE.max_height_render += event.wheel.y;

                    if(RE.max_height_render > world.max_block_coord.z)
                        RE.max_height_render = world.max_block_coord.z;
                    else if(RE.max_height_render < 0)
                        RE.max_height_render = 0;

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
            
            case SDL_MOUSEBUTTONDOWN :
                if(event.button.button == SDL_BUTTON_RIGHT)
                {
                    if(RE.highlight_mode == HIGHLIGHT_REMOVE)
                        GameEvent.add_event(GAME_EVENT_SINGLE_BLOCK_MOD, (coord3D)RE.highlight_wcoord, BLOCK_EMPTY);
                    
                    else if(RE.highlight_mode == HIGHLIGHT_PLACE || RE.highlight_mode == HIGHLIGHT_PLACE_ALT) {
                        GameEvent.add_event(GAME_EVENT_SINGLE_BLOCK_MOD, (coord3D)RE.highlight_wcoord, Current_block);

                        if (Current_block == BLOCK_WATER) {
                            block_coordonate bcoord = world.convert_wcoord(RE.highlight_wcoord.x, RE.highlight_wcoord.y, RE.highlight_wcoord.z);
                            PhysicsEventWater *pe = new PhysicsEventWater(&world, &physics_engine, &GameEvent, bcoord);
                            physics_engine.add_event(pe);
                        }
                            
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
    }

    GameEvent.game_is_running = false;
    GameEvent.handle();

    SDL_WaitThread(RE.SecondaryThread, NULL);

    GameEvent.drop_all_nfs_event();
    SDL_CondSignal(GameEvent.new_nfs_event);
    SDL_WaitThread(GameEvent.NFS_Thread, NULL);

    return state;
}