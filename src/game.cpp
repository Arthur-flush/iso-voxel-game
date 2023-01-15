#include <game.hpp>

Game::Game(GPU_Target* _screen) : RE(world), GameEvent(RE)
{
    state = STATE_QUIT;
    init(_screen);
}

void Game::init(GPU_Target* _screen)
{
    for(int i = TEXTURE_MIN_ID; i < TEXTURE_MAX_NUMBER; i++)
        RE.Textures[i] = std::make_shared<Texture>(i, RE.renderer);

    // chunk_coordonate world_size = {256, 256, 32};
    // chunk_coordonate world_size = {32, 32, 12};
    chunk_coordonate world_size = {128, 128, 16};
    // chunk_coordonate world_size = {64, 64, 12};
    world.init(world_size.x, world_size.y, world_size.z);
    RE.projection_grid.init_pos(world_size.x*CHUNK_SIZE, world_size.y*CHUNK_SIZE, world_size.z*CHUNK_SIZE);

    state = STATE_MAIN;
    Current_block = BLOCK_RED;

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
    // world 3
    GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.shader.get_location("water"), 4);
    RE.shader.deactivate();

    RE.max_height_render = world.max_block_coord.z;

    world.world_view_position = 0;
    /// a remplacer plus tard ///
    std::cout << "game init : generating world... ";
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
                    world.chunk[x][y][z].block[i][j][k].id = BLOCK_SAND;
            }

            if(wz < 42 && !world.chunk[x][y][z].block[i][j][k].id)
                world.chunk[x][y][z].block[i][j][k].id = BLOCK_WATER;

            if(wz < 42 && (wx == world.max_block_coord.x-1 || wy == world.max_block_coord.y-1))
                world.chunk[x][y][z].block[i][j][k].id = BLOCK_DEBUG;
            
            if(wx == 150)
                world.chunk[x][y][z].block[i][j][k].id = BLOCK_DEBUG;

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
    std::cout << "finished !\n";


    // RE.refresh_sprite_size();

    // RE.refresh_all_block_visible();

    // RE.projection_grid.refresh_all_identical_line();

    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

    // RE.refresh_all_block_visible2();
    // RE.refresh_all_render_flags2();
    RE.projection_grid.refresh_all_identical_line();

    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){RE.window.size.x/2, RE.window.size.y/2});
    // GameEvent.add_event(GAME_EVENT_INIT_WORLD_RENDER_FLAGS);
    GPU_AddDepthBuffer(RE.screen2);
    GPU_SetDepthFunction(RE.screen2, GPU_LEQUAL);
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
                            GameEvent.add_event(GAME_EVENT_HIGHLIGHT_CHANGE, {{-1, -1, -1}, -1, -1, -1});
                            // RE.highlight_coord = {-1, -1, -1};
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
                            // time the save using chrono
                            // Uint64 start = Get_time_ms();

                            status = world.save_to_file("test.worldsave");

                            if(status == 0)
                            {
                                std::cout << "world saved !\n";
                            }
                            else
                            {
                                std::cout << "failded to save world :(\n";
                            }
                            break;
                        }

                        case SDLK_F7: {
                            Uint64 start = Get_time_ms();

                            status = world.load_from_file("test.worldsave");

                            if(status == 0) 
                            {
                                std::cout << "world load successfully :)\n";

                                RE.world = world;
                                world.compress_all_chunks();


                                GameEvent.drop_all_nfs_event();

                                RE.projection_grid.clear();
                                RE.projection_grid.save_curr_interval();
                                // GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
                                RE.refresh_pg_onscreen();
                                RE.refresh_pg_block_visible();
                                GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                                GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

                            }
                            else
                                std::cout << "world load failed ._. !\n";
                            break;
                        }

                        case SDLK_r :
                            RE.projection_grid.save_curr_interval();
                            RE.refresh_pg_onscreen();
                            break;

                        case SDLK_LEFT :
                            Current_block = Current_block == BLOCK_SAND ? 0 : Current_block;
                            Current_block = (Current_block+1)%5;
                            Current_block = Current_block == 0 ? BLOCK_SAND : Current_block;
                            RE.current_block_tmp = Current_block;
                            break;
                        
                        case SDLK_RIGHT :
                            Current_block = Current_block == BLOCK_SAND ? 5 : Current_block;
                            Current_block = (Current_block-1)%5;
                            Current_block = Current_block == 0 ? BLOCK_SAND : Current_block;
                            RE.current_block_tmp = Current_block;
                            break;

                        case SDLK_TAB :
                            std::cout << "sprite counter : " << RE.sprite_counter << '\n';
                            std::cout << "scale : 1/" << 1/RE.window.scale << '\n';
                            std::cout << "block_size : " << RE.block_onscreen_size << '\n';
                            std::cout << "World view position : " << world.world_view_position << '\n';
                            break;

                        case SDLK_RALT :
                            RE.debug = !RE.debug;
                            break;

                        default :
                            break;
                    }
                break;

            case SDL_MOUSEMOTION :
                // std::cout << event.motion.state << '\n';
                if(event.motion.state == SDL_BUTTON_LMASK)
                {
                    // std::cout << "Processing game CAMERA_MOUVEMENT event...\n";
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

                    // RE.projection_grid.save_curr_interval();
                    // GameEvent.drop_all_nfs_event();
                    // GameEvent.add_nfs_event(NFS_OP_PG_MHR);
                    // GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                    // GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                    
                    // RE.projection_grid.clear();
                    // RE.projection_grid.save_curr_interval();
                    // GameEvent.drop_all_nfs_event();
                    // GameEvent.add_nfs_event(NFS_OP_PG_MHR);

                    // GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                    // GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

                    // RE.refresh_all_block_visible2();
                    // RE.refresh_all_render_flags2();
                    // RE.projection_grid.refresh_all_identical_line();

                    // GameEvent.drop_all_nfs_event();
                    // GameEvent.add_nfs_event(NFS_OP_PG_MHR);
                    // GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                    // GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                }

                break;
            
            case SDL_MOUSEBUTTONDOWN :
                if(event.button.button == SDL_BUTTON_RIGHT)
                {
                    if(RE.highlight_mode == HIGHLIGHT_REMOVE)
                        GameEvent.add_event(GAME_EVENT_SINGLE_CHUNK_MOD, (coord3D)RE.highlight_wcoord, BLOCK_EMPTY);
                    
                    else if(RE.highlight_mode == HIGHLIGHT_PLACE || RE.highlight_mode == HIGHLIGHT_PLACE_ALT)
                        GameEvent.add_event(GAME_EVENT_SINGLE_CHUNK_MOD, (coord3D) RE.highlight_wcoord, Current_block);
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