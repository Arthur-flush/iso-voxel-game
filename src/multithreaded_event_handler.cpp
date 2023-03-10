#include <multithreaded_event_handler.hpp>

std::string format(unsigned long long i) {
  char buffer[128]; // can be adapted more tightly with std::numeric_limits

  char* p = buffer + 128;
  *(--p) = '\0';

  unsigned char count = 0;
  while (i != 0) {
    *(--p) = '0' + (i % 10);
    i /= 10;

    if (++count == 3) { count = 0; *(--p) = ' '; }
  }

  return p;
}

Multithreaded_Event_Handler::Multithreaded_Event_Handler(Render_Engine &_RE) : RE{_RE}
{
    PE = NULL;
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

void Multithreaded_Event_Handler::add_event(game_event *new_event)
{
    int y = new_event->id; 
    event_queue.push(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id)
{
    game_event* new_event = new game_event;

    new_event->id = _id;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const float _scale)
{
    game_event* new_event = new game_event;

    new_event->id = _id;
    new_event->data.scale = _scale;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const pixel_coord _target)
{
    game_event* new_event = new game_event;

    new_event->id = _id;
    new_event->data.target = _target;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const coord3D coord)
{
    game_event* new_event = new game_event;

    new_event->id = _id;
    new_event->data.coord1.chunk = coord;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate coord)
{
    game_event* new_event = new game_event;

    new_event->id = _id;
    new_event->data.coord1 = coord;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate coord, Uint16 _blockid)
{
    game_event* new_event  = new game_event;

    new_event->id = _id;
    new_event->data.coord1 = coord;
    new_event->data.blockid = _blockid;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const world_coordonate coord, Uint16 _blockid)
{
    game_event* new_event  = new game_event;

    new_event->id = _id;
    new_event->data.wcoord1 = coord;
    new_event->data.blockid = _blockid;

    add_event(new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate _coord1, const block_coordonate _coord2)
{
    game_event* new_event  = new game_event;

    new_event->id = _id;
    new_event->data.coord1 = _coord1;
    new_event->data.coord1 = _coord2;

    add_event(new_event);
}

void Multithreaded_Event_Handler::handle()
{
    SDL_LockMutex(init_cond);

    SecondaryThread_opcode = 0;

    bool refresh_identical_line = false;

    while(!event_queue.empty())
    {
        game_event *event = event_queue.front();

        switch(event->id)
        {
        case GAME_EVENT_NEWSCALE :

            RE.window.scale = event->data.scale;

            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

            break;

        case GAME_EVENT_ADDSCALE :

            if(event->data.scale > 0)
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
            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            break;
        
        case GAME_EVENT_CAMERA_MOUVEMENT :

            RE.target.x += event->data.target.x;
            RE.target.y += event->data.target.y;
            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;
            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            break;

        case GAME_EVENT_SINGLE_BLOCK_MOD :
        {
            // std::cout << "attempting to lock world mutex in single block mod\n"; 
            PE->world_mutex.lock();
            // std::cout << "world mutex lock in single block mod" << std::endl;

            if(RE.highlight_wcoord2.x == -1)
            {
                if(RE.world.modify_block(event->data.wcoord1, event->data.blockid))
                {
                    block_coordonate bc = RE.world.convert_wcoord(event->data.wcoord1.x, event->data.wcoord1.y, event->data.wcoord1.z);

                    RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);
                    RE.refresh_block_render_flags(bc.chunk, bc.x, bc.y, bc.z);

                    // SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;
                }
            }
            else if(RE.highlight_wcoord.x != -1)
            {
                int xbeg = RE.highlight_wcoord.x;
                int xend = RE.highlight_wcoord2.x;

                int ybeg = RE.highlight_wcoord.y;
                int yend = RE.highlight_wcoord2.y;

                int zbeg = RE.highlight_wcoord.z;
                int zend = RE.highlight_wcoord2.z;


                if(RE.highlight_wcoord.x > RE.highlight_wcoord2.x)
                {
                    xbeg = RE.highlight_wcoord2.x;
                    xend = RE.highlight_wcoord.x;
                }

                if(RE.highlight_wcoord.y > RE.highlight_wcoord2.y)
                {
                    ybeg = RE.highlight_wcoord2.y;
                    yend = RE.highlight_wcoord.y;
                }

                if(RE.highlight_wcoord.z > RE.highlight_wcoord2.z)
                {
                    zbeg = RE.highlight_wcoord2.z;
                    zend = RE.highlight_wcoord.z;
                }

                // Uint64 start = Get_time_ms();
                
                Uint64 estimate_itcounter = (Uint64)(yend-ybeg+1)*(Uint64)(xend-xbeg+1)*(Uint64)(zend-zbeg+1);

                // Uint64 itcounter = 0;

                if(estimate_itcounter < 0) // DEBUG: UNDO WHEN DONE DEBUGING
                // sorry if I forget :pleading_face:
                {
                    for(int y = ybeg; y <= yend; y ++)
                    {
                        for(int x = xbeg; x <= xend; x ++)
                        for(int z = zbeg; z <= zend; z ++)
                        {
                            if(RE.world.modify_block({x, y, z}, event->data.blockid))
                            {
                                block_coordonate bc = RE.world.convert_wcoord(x, y, z);

                                RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);
                                // RE.refresh_block_render_flags(bc.chunk, bc.x, bc.y, bc.z);


                                int _xbeg = x > 0 ? x-1 : 0;
                                int _xend = x < RE.world.max_block_coord.x ? x+1 : RE.world.max_block_coord.x;

                                int _ybeg = y > 0 ? y-1 : 0;
                                int _yend = y < RE.world.max_block_coord.y ? y+1 : RE.world.max_block_coord.y;

                                int _zbeg = z > 0 ? z-1 : 0;
                                int _zend = z < RE.world.max_block_coord.z ? z+1 : RE.world.max_block_coord.z;

                                for(int _x = _xbeg; _x <= _xend; _x++)
                                for(int _y = _ybeg; _y <= _yend; _y++)
                                for(int _z = _zbeg; _z <= _zend; _z++)
                                {
                                    coord3D c = RE.projection_grid.convert_wcoord(_x, _y, _z);
                                    RE.set_block_renderflags(c.x, c.y, c.z);
                                }
                            }
                        }

                        // RE.refresh_line_shadows(0, xend, y, zend);
                    }

                    RE.refresh_line_shadows(RE.highlight_wcoord, RE.highlight_wcoord2);
                }
                else
                {
                    if(RE.highlight_type != HIGHLIGHT_VOLUME)
                    {
                        for(int y = ybeg; y <= yend; y ++)
                        for(int x = xbeg; x <= xend; x ++)
                        for(int z = zbeg; z <= zend; z ++)
                        {
                            RE.world.modify_block({x, y, z}, event->data.blockid);
                        }
                    }
                    else
                    {

                        int chunk_xbeg = (xbeg + CHUNK_SIZE - (xbeg%CHUNK_SIZE))/CHUNK_SIZE;
                        chunk_xbeg = xbeg%CHUNK_SIZE ? chunk_xbeg : xbeg/CHUNK_SIZE;
                        int chunk_xend = ((xend+1) - ((xend+1)%CHUNK_SIZE))/CHUNK_SIZE;

                        int chunk_ybeg = (ybeg + CHUNK_SIZE - (ybeg%CHUNK_SIZE))/CHUNK_SIZE;
                        chunk_ybeg = ybeg%CHUNK_SIZE ? chunk_ybeg : ybeg/CHUNK_SIZE;
                        int chunk_yend = ((yend+1) - ((yend+1)%CHUNK_SIZE))/CHUNK_SIZE;

                        int chunk_zbeg = (zbeg + CHUNK_SIZE - (zbeg%CHUNK_SIZE))/CHUNK_SIZE;
                        chunk_zbeg = zbeg%CHUNK_SIZE ? chunk_zbeg : zbeg/CHUNK_SIZE;
                        int chunk_zend = ((zend+1) - ((zend+1)%CHUNK_SIZE))/CHUNK_SIZE;

                        int chunk_xbeg_wvp = chunk_xbeg;
                        int chunk_xend_wvp = chunk_xend;
                        int chunk_ybeg_wvp = chunk_ybeg;
                        int chunk_yend_wvp = chunk_yend;
                        int chunk_zbeg_wvp = chunk_zbeg;
                        int chunk_zend_wvp = chunk_zend;

                        if(RE.world.world_view_position%2)
                        {
                            chunk_xbeg_wvp = chunk_ybeg;
                            chunk_xend_wvp = chunk_yend;

                            chunk_ybeg_wvp = chunk_xbeg;
                            chunk_yend_wvp = chunk_xend;
                        }

                        if(RE.world.world_view_position == 1 || RE.world.world_view_position == 2)
                        {
                            chunk_ybeg_wvp = RE.world.max_chunk_coord.y-chunk_ybeg_wvp;
                            chunk_yend_wvp = RE.world.max_chunk_coord.y-chunk_yend_wvp;

                            int tmp = chunk_ybeg_wvp+1;
                            chunk_ybeg_wvp = chunk_yend_wvp+1;
                            chunk_yend_wvp = tmp;
                        }

                        if(RE.world.world_view_position == 3 || RE.world.world_view_position == 2)
                        {
                            chunk_xbeg_wvp = RE.world.max_chunk_coord.x-chunk_xbeg_wvp;
                            chunk_xend_wvp = RE.world.max_chunk_coord.x-chunk_xend_wvp;

                            int tmp = chunk_xbeg_wvp+1;
                            chunk_xbeg_wvp = chunk_xend_wvp+1;
                            chunk_xend_wvp = tmp;
                        }

                        int itcounter = 0;
                        for(int cx = chunk_xbeg_wvp; cx < chunk_xend_wvp; cx++)
                        for(int cy = chunk_ybeg_wvp; cy < chunk_yend_wvp; cy++)
                        for(int cz = chunk_zbeg_wvp; cz < chunk_zend_wvp; cz++)
                        {
                            // itcounter += CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
                            memset(RE.world.chunks[cx][cy][cz].blocks, event->data.blockid, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(block));
                            RE.world.chunks[cx][cy][cz].compress_value = event->data.blockid;
                            
                            chunk_coordonate c = {cx, cy, cz};
                            PE->add_event(PHYSICS_EVENT_WATER_CHECK_CHUNK, &c);
                            itcounter ++;
                            // std::cout << "adding event for chunk " << cx << " " << cy << " " << cz << std::endl;
                        }

                        // std::cout << "memset chunks: " << itcounter << std::endl;
                        // std::cout << "chunk_xend_wvp: " << chunk_xend_wvp << std::endl;
                        // std::cout << "chunk_yend_wvp: " << chunk_yend_wvp << std::endl;
                        // std::cout << "chunk_zend_wvp: " << chunk_zend_wvp << std::endl;

                        // std::cout << "xend: " << xend << std::endl;
                        // std::cout << "yend: " << yend << std::endl;
                        // std::cout << "zend: " << zend << std::endl;

                        for(int z = zbeg; z <= zend; z ++)
                            {

                                for(int y = ybeg; y <= yend; y ++)
                                {

                                    for(int x = xbeg; x <= xend; x ++)
                                    {
                                        if(z >= chunk_zbeg*CHUNK_SIZE && z < chunk_zend*CHUNK_SIZE)
                                            if(y >= chunk_ybeg*CHUNK_SIZE && y < chunk_yend*CHUNK_SIZE)
                                                if(x == chunk_xbeg*CHUNK_SIZE)
                                                    x = chunk_xend*CHUNK_SIZE;

                                        RE.world.modify_block({x, y, z}, event->data.blockid);
                                        // itcounter ++;
                                    }
                                    
                                }

                            }
                    }

                    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

                    drop_all_nfs_event();
                    RE.projection_grid.clear();
                    RE.projection_grid.save_curr_interval();
                    RE.refresh_pg_onscreen();
                    RE.refresh_pg_block_visible();
                    add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                    add_nfs_event(NFS_OP_ALL_RENDER_FLAG); 
                }

                RE.highlight_wcoord2 = {-1, -1, -1};

                // Uint64 end = Get_time_ms();
                // std::locale::global(std::locale(""));
                // std::cout << "\nPosing " << format(estimate_itcounter) << " blocks in " << end-start << " ms.";
                // std::cout << "\nActually posed " << format(itcounter) << " blocks \n";
                // SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;  
            }

            // std::cout << "world mutex unlock in game event single block mod" << std::endl;
            PE->world_mutex.unlock();
            RE.projection_grid.refresh_all_identical_line(); 
            break;
        }

        case GAME_EVENT_SINGLE_BLOCK_MOD_ALT :
            {
                PE->world_mutex.lock();
                block_coordonate bc = event->data.coord1;

                block *b = &RE.world.
                            chunks
                            [bc.chunk.x]
                            [bc.chunk.y]
                            [bc.chunk.z].
                            blocks
                            [bc.x]
                            [bc.y]
                            [bc.z];

                if(!b->id)
                {
                    b->id = event->data.blockid;

                    RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);

                    RE.world.compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);

                    coord3D wc = RE.world.convert_coord(bc);

                    RE.refresh_line_shadows(wc.x, wc.x, wc.y, wc.z);

                    coord3D pgc = RE.projection_grid.convert_wcoord(wc.x, wc.y, wc.z);
                    RE.set_block_renderflags(pgc.x, pgc.y, pgc.z);

                    refresh_identical_line = true;
                }

                PE->world_mutex.unlock();
                break;
            }

        case GAME_EVENT_INIT_WORLD_RENDER_FLAGS :
            SecondaryThread_opcode |= STHREAD_OP_ALL_RENDER_FLAG;

        default:
            break;
        }

        event_queue.pop();
        delete event;
    }

    SDL_UnlockMutex(init_cond);

    if(refresh_identical_line)
    {
        RE.projection_grid.refresh_all_identical_line(); 
    }

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
    while(nfs_event_queue.size() > 1)
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

                MEH->is_NFS_reading_to_wpg = true;
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
                    MEH->RE.refresh_pg_block_visible();
                    MEH->RE.projection_grid.refresh_all_identical_line();
                    break;

                case NFS_OP_PG_MHR :
                    MEH->RE.refresh_pg_MHR();
                    break;

                default:
                    break;
                }
                MEH->is_NFS_reading_to_wpg = false;

                MEH->nfs_event_queue.pop();
            }
        }

        SDL_UnlockMutex(MEH->nfs_mut);
    }

    return 0;
}