#ifndef MULTITHREADED_EVENT_HANDLER_HPP
#define MULTITHREADED_EVENT_HANDLER_HPP

#include <queue>

#include <SDL2/SDL.h>
#include <render_engine.hpp>

// "scale" must be given as a float
#define GAME_EVENT_NEWSCALE                  0
// "scale" must be given as a float
#define GAME_EVENT_ADDSCALE                  1
// add given pixel_coord to the Render_Engine::target
#define GAME_EVENT_CAMERA_MOUVEMENT          2
// replace the world's block at the given coord3D coordonate with the given block_id
// automaticly translate the coords to world view position
// automaticly check for any error or bad resquest
// automaticly refresh the projection grid (shadows, visible blocks and identical_lines)
#define GAME_EVENT_SINGLE_BLOCK_MOD          5

// replace the world's block at the given block_coordonate with the given block_id
// does NOT translate the coords to world view position
// does NOT check for any error or bad resquest
// automaticly refresh world's display & chunk compression
// ==> use it when you have to modify a potentialy large quantity of block, but not too frequently
#define GAME_EVENT_SINGLE_BLOCK_MOD_ALT      6
#define GAME_EVENT_INIT_WORLD_RENDER_FLAGS   7

#define STHREAD_OP_PG_BLOCK_VISIBLE     0b00000010
#define STHREAD_OP_ALL_BLOCK_VISBLE     0b00000100
#define STHREAD_OP_ALL_RENDER_FLAG      0b00001000
#define STHREAD_OP_SINGLE_CHUNK_POS     0b00010000
#define STHREAD_OP_SINGLE_BLOCK_VISBLE  0b00100000
#define STHREAD_OP_SINGLE_RENDER_FLAGS  0b01000000

#define NFS_OP_NONE                     0
#define NFS_OP_ALL_BLOCK_VISIBLE        1
#define NFS_OP_ALL_RENDER_FLAG          2
#define NFS_OP_PG_ONSCREEN              3
#define NFS_OP_PG_MHR                   4


// Function that will be run on the Secondary thread
// It is frame sensitive, so the game will wait for this 
// function to signal Multithreaded_Event_Handler::new_frame_to_render 
// before passing to the next frame
int SecondaryThread_operations(void *);

// Function that will be run on the Non Frame Sensitive Operations thread
int NFS_operations(void *);

struct game_event_aditional_data
{
    float scale;
    pixel_coord target;

    world_coordonate wcoord1;
    world_coordonate wcoord2;

    block_coordonate coord1;
    block_coordonate coord2;
    Uint16 blockid;
};

struct game_event
{
    int id;
    game_event_aditional_data data;
};

class Multithreaded_Event_Handler
{
    private :
        friend int SecondaryThread_operations(void*);
        friend int NFS_operations(void*);
        Render_Engine &RE;
        std::queue<game_event> event_queue;

        SDL_cond *new_frame_to_render;
        int SecondaryThread_opcode;

        game_event_aditional_data STO_data;

        /********* NFS OP ************/
        SDL_mutex *nfs_mut;
        std::queue<int> nfs_event_queue;
        /****************************/

    public :
        Multithreaded_Event_Handler(Render_Engine&);
        ~Multithreaded_Event_Handler();

        bool game_is_running;
        SDL_mutex *init_cond;
        SDL_cond  *secondary_frame_op_finish;

        /********* Event OP ************/

        // Add an event
        void add_event(game_event&);
        void add_event(const int);
        void add_event(const int, const float);
        void add_event(const int, const pixel_coord);
        void add_event(const int, const coord3D);
        void add_event(const int, const block_coordonate);
        void add_event(const int, const block_coordonate, Uint16);
        void add_event(const int, const world_coordonate, Uint16);
        void add_event(const int, const block_coordonate, const block_coordonate);
        
        
        // Handle all gamevent previously pushed
        // Prepare all secondary thread operation and send them to the corresponding thread
        // It does not include nfs events
        void handle();
        /****************************/

        /********* NFS OP ************/


        SDL_Thread *NFS_Thread;
        SDL_cond *new_nfs_event;

        bool is_NFS_reading_to_wpg;

        // Queue a new Non Frame Sensitive event for the corresponding thread
        void add_nfs_event(const int nfs_event_id);

        // Drop all Non Frame Sensitive operation from the corresponding thread
        void drop_all_nfs_event();


        /****************************/
};


#endif