#include <multithreaded_event_handler.hpp>

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

            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

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

            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            break;
        
        case GAME_EVENT_CAMERA_MOUVEMENT :

            RE.target.x += event.data.target.x;
            RE.target.y += event.data.target.y;
            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            break;

        case GAME_EVENT_SINGLE_BLOCK_MOD :
            
            if(RE.world.modify_block(event.data.wcoord1, event.data.blockid))
            {
                block_coordonate bc = RE.world.convert_wcoord(event.data.wcoord1.x, event.data.wcoord1.y, event.data.wcoord1.z);

                RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);
                RE.refresh_block_render_flags(bc.chunk, bc.x, bc.y, bc.z);

                RE.projection_grid.refresh_all_identical_line();  

                SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;
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
            SDL_CondWait(MEH->new_frame_to_render, MEH->init_cond);

            ///////////////////// BLOCK VISIBLE  //////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_ALL_BLOCK_VISBLE)
                MEH->RE.refresh_all_block_visible2();

            else if(MEH->SecondaryThread_opcode & STHREAD_OP_SINGLE_BLOCK_VISBLE)
                MEH->RE.refresh_block_visible(MEH->STO_data.coord1.chunk, MEH->STO_data.coord1.x, MEH->STO_data.coord1.y, MEH->STO_data.coord1.z);

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

            /////////////////////// PG BLOCK VISIBLE  ////////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_PG_BLOCK_VISIBLE)
            {
                MEH->RE.refresh_pg_block_visible();
            }
        }

        SDL_UnlockMutex(MEH->init_cond);
        SDL_CondSignal(MEH->secondary_frame_op_finish);
    }

    return 0;
}

void Multithreaded_Event_Handler::add_nfs_event(const int nfs_event_id)
{
    nfs_event_queue.push(nfs_event_id);
    SDL_CondSignal(new_nfs_event);
}

void Multithreaded_Event_Handler::drop_all_nfs_event()
{
    while(!nfs_event_queue.empty())
        nfs_event_queue.pop();
}

int NFS_operations(void *data)
{
    Multithreaded_Event_Handler* MEH = (Multithreaded_Event_Handler*)data;

    int event;
    
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

    return 0;
}