#include <game.hpp>

Multithreaded_Event_Handler::Multithreaded_Event_Handler(Render_Engine &_RE) : RE{_RE}
{
    init_cond = SDL_CreateMutex();
    new_frame_to_render = SDL_CreateCond();
    secondary_frame_op_finish = SDL_CreateCond();

    nfs_mut = SDL_CreateMutex();
    new_nfs_event = SDL_CreateCond();

    game_is_running = true;
    RE.SecondaryThread = SDL_CreateThread(SecondaryThread_operations, "iso2", this);
    NFS_Thread = SDL_CreateThread(NFS_operations, "iso3", this);
}

Multithreaded_Event_Handler::~Multithreaded_Event_Handler()
{
    SDL_DestroyMutex(init_cond);
    SDL_DestroyMutex(nfs_mut);

    SDL_DestroyCond(new_frame_to_render);
    SDL_DestroyCond(secondary_frame_op_finish);
    SDL_DestroyCond(new_nfs_event);
}

void Multithreaded_Event_Handler::add_event(game_event &new_event)
{
    event_queue.push(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id)
{
    game_event new_event;

    new_event.id = _id;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const float _scale)
{
    game_event new_event;

    new_event.id = _id;
    new_event.data.scale = _scale;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const pixel_coord _target)
{
    game_event new_event;

    new_event.id = _id;
    new_event.data.target = _target;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const coord3D coord)
{
    game_event new_event;

    new_event.id = _id;
    new_event.data.coord1.chunk = coord;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate coord)
{
    game_event new_event;

    new_event.id = _id;
    new_event.data.coord1 = coord;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const world_coordonate coord, Uint16 _blockid)
{
    game_event new_event;

    new_event.id = _id;
    new_event.data.wcoord1 = coord;
    new_event.data.blockid = _blockid;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate _coord1, const block_coordonate _coord2)
{
    game_event new_event;

    new_event.id = _id;
    new_event.data.coord1 = _coord1;
    new_event.data.coord1 = _coord2;

    add_event(new_event);
}

void Multithreaded_Event_Handler::handle()
{
    SDL_LockMutex(init_cond);

    SecondaryThread_opcode = 0;

    while(!event_queue.empty())
    {
        game_event &event = event_queue.front();

        switch(event.id)
        {
        case GAME_EVENT_NEWSCALE :

            RE.window.scale = event.data.scale;

            SecondaryThread_opcode |= STHREAD_OP_BLOCK_ONSCREEN;
            SecondaryThread_opcode |= STHREAD_OP_ALL_CHUNK_POS;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

            break;

        case GAME_EVENT_ADDSCALE :

            if(event.data.scale > 0)
            {
                if(RE.window.scale < MAX_FLOAT_SCALE)
                {
                    RE.window.scale *= 2;
                    RE.target.x = RE.target.x*2 - (RE.window.size.x*0.5);
                    RE.target.y = RE.target.y*2 - (RE.window.size.y*0.5);
                }
            }
            else if(RE.window.scale > MIN_FLOAT_SCALE)
            {
                RE.window.scale /= 2;
                RE.target.x = (RE.target.x)/2 + (RE.window.size.x*0.25);
                RE.target.y = (RE.target.y)/2 + (RE.window.size.y*0.25);
            }

            SecondaryThread_opcode |= STHREAD_OP_BLOCK_ONSCREEN;
            SecondaryThread_opcode |= STHREAD_OP_ALL_CHUNK_POS;

            // std::cout << "Scale : " << 1.0/RE.window.scale << '\n'; 
            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            break;
        
        case GAME_EVENT_CAMERA_MOUVEMENT :

            RE.target.x += event.data.target.x;
            RE.target.y += event.data.target.y;
            SecondaryThread_opcode |= STHREAD_OP_ALL_CHUNK_POS;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

            // system("cls");
            // std::cout << "PROJECTION GRID VISIBLE FRAGS :\n";
            // std::cout << RE.projection_grid.visible_frags[2][0].beg << ' ';
            // std::cout << RE.projection_grid.visible_frags[2][0].end << '\n';
            // std::cout << RE.projection_grid.visible_frags[2][1].beg << ' ';
            // std::cout << RE.projection_grid.visible_frags[2][1].end << '\n';
            break;
        
        case GAME_EVENT_SINGLE_CHUNK_POS_REFRESH :

            if(event.data.coord1.chunk.x >= RE.world.min_chunk_coord.x &&
               event.data.coord1.chunk.y >= RE.world.min_chunk_coord.y &&
               event.data.coord1.chunk.z >= RE.world.min_chunk_coord.z &&
               event.data.coord1.chunk.x <= RE.world.max_chunk_coord.x &&
               event.data.coord1.chunk.y <= RE.world.max_chunk_coord.y &&
               event.data.coord1.chunk.z <= RE.world.max_chunk_coord.z)
               {
                STO_data.coord1.chunk = event.data.coord1.chunk;   
                SecondaryThread_opcode |= STHREAD_OP_SINGLE_CHUNK_POS;
               }

            break;
        
        case GAME_EVENT_HIGHLIGHT_CHANGE : // old
            {
                RE.highlight_coord  = event.data.coord1;
                RE.highlight_wcoord = {event.data.coord1.x+event.data.coord1.chunk.x*CHUNK_SIZE,
                                       event.data.coord1.y+event.data.coord1.chunk.y*CHUNK_SIZE,
                                       event.data.coord1.z+event.data.coord1.chunk.z*CHUNK_SIZE
                                      };
            }
            break;

        case GAME_EVENT_SINGLE_CHUNK_MOD :
            
            if(RE.world.modify_block(event.data.wcoord1, event.data.blockid))
            {
                block_coordonate bc = RE.world.convert_wcoord(event.data.wcoord1.x, event.data.wcoord1.y, event.data.wcoord1.z);

                RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);
                RE.refresh_block_render_flags(bc.chunk, bc.x, bc.y, bc.z);

                RE.projection_grid.refresh_all_identical_line();  

                SecondaryThread_opcode |= STHREAD_OP_ALL_CHUNK_POS;
            }
            
            break;

        case GAME_EVENT_INIT_WORLD_RENDER_FLAGS :
            SecondaryThread_opcode |= STHREAD_OP_ALL_RENDER_FLAG;

        default:
            break;
        }

        event_queue.pop();
    }

    SDL_UnlockMutex(init_cond);

    if(SecondaryThread_opcode || !game_is_running)
    {
        // std::cout << "MEH class : Signaling new_frame_to render\n";
        SDL_CondSignal(new_frame_to_render);
    }
}

int SecondaryThread_operations(void *data)
{
    Multithreaded_Event_Handler* MEH = (Multithreaded_Event_Handler*)data;

    while(MEH->game_is_running)
    {
        SDL_LockMutex(MEH->init_cond);

        if(MEH->game_is_running)
        {
            // std::cout << "STO fnct : Waiting for new_frame_to render\n";
            SDL_CondWait(MEH->new_frame_to_render, MEH->init_cond);
            // std::cout << "STO fnct : Receiving signal new_frame_to render\n";

            if(MEH->SecondaryThread_opcode & STHREAD_OP_BLOCK_ONSCREEN)
                MEH->RE.refresh_sprite_size();

            ///////////////////// BLOCK VISIBLE  //////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_ALL_BLOCK_VISBLE)
                MEH->RE.refresh_all_block_visible2();

            else if(MEH->SecondaryThread_opcode & STHREAD_OP_SINGLE_BLOCK_VISBLE)
            {
                MEH->RE.refresh_block_visible(MEH->STO_data.coord1.chunk, MEH->STO_data.coord1.x, MEH->STO_data.coord1.y, MEH->STO_data.coord1.z);
            }
            ////////////////////////////////////////////////////////////

            ////////////////////// RENDER FLAGS  ///////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_ALL_RENDER_FLAG)
            {
                MEH->RE.refresh_all_render_flags2();
                MEH->RE.projection_grid.refresh_all_identical_line();
            }

            else if(MEH->SecondaryThread_opcode & STHREAD_OP_SINGLE_RENDER_FLAGS)
            {
                MEH->RE.refresh_block_render_flags(MEH->STO_data.coord1.chunk, MEH->STO_data.coord1.x, MEH->STO_data.coord1.y, MEH->STO_data.coord1.z);
                MEH->RE.projection_grid.refresh_all_identical_line();
            }
            ////////////////////////////////////////////////////////////

            /////////////////////// CHUNK POS  ////////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_ALL_CHUNK_POS)
            {
                MEH->RE.refresh_pg_block_visible();
            }
            
            // else if(MEH->SecondaryThread_opcode & STHREAD_OP_SINGLE_CHUNK_POS)
            //     MEH->RE.refresh_chunk_pos(MEH->STO_data.coord1.chunk);
            ////////////////////////////////////////////////////////////
        }

        // GPU_Rect src_rect = {0, 256, MOSAIC_TEXTURE_SIZE, MOSAIC_TEXTURE_SIZE};
        // GPU_Rect dst_rect = {0, 0, 0, 0};
        // GPU_SetColor(MEH->RE.Textures[MOSAIC]->ptr, {128, 128, 128, 0});
        // GPU_BlitRect(MEH->RE.Textures[MOSAIC]->ptr, &src_rect, MEH->RE.screen, &dst_rect);

        SDL_UnlockMutex(MEH->init_cond);
        SDL_CondSignal(MEH->secondary_frame_op_finish);
        // std::cout << "STO fnct : Signaling secondary_frame_op_finish\n";
    }

    // std::cout << "STO fnct : closing secondary thread\n";

    return 0;
}

void Multithreaded_Event_Handler::add_nfs_event(const int nfs_event_id)
{
    nfs_event_queue.push(nfs_event_id);
    SDL_CondSignal(new_nfs_event);
}

void Multithreaded_Event_Handler::drop_all_nfs_event()
{
    // stop_nfs_op = false;

    while(!nfs_event_queue.empty())
        nfs_event_queue.pop();
}

int NFS_operations(void *data)
{
    Multithreaded_Event_Handler* MEH = (Multithreaded_Event_Handler*)data;

    int event;
    
    // std::cout << "\n===> Launching NFS OP Thread\n";
    while(MEH->game_is_running)
    {
        SDL_LockMutex(MEH->nfs_mut);

        if(MEH->game_is_running)
        {
            SDL_CondWait(MEH->new_nfs_event, MEH->nfs_mut);

            event = NFS_OP_NONE;


            while(!MEH->nfs_event_queue.empty())
            {
                event = MEH->nfs_event_queue.front();
                MEH->nfs_event_queue.pop();

                switch (event)
                {
                case NFS_OP_ALL_BLOCK_VISIBLE :
                    MEH->RE.refresh_all_block_visible2();
                    break;
                
                case NFS_OP_ALL_RENDER_FLAG :
                    MEH->RE.refresh_all_render_flags2();
                    MEH->RE.projection_grid.refresh_all_identical_line();
                    break;

                case NFS_OP_PG_ONSCREEN  : 
                    MEH->RE.refresh_pg_onscreen();
                    break;

                case NFS_OP_PG_MHR :
                    MEH->RE.refresh_pg_MHR();
                    break;

                default:
                    break;
                }
            }
        }

        SDL_UnlockMutex(MEH->nfs_mut);
    }

    // std::cout <<  "\n===> Closing NFS OP Thread\n";

    return 0;
}