#ifndef MULTITHREADED_EVENT_HANDLER_HPP
#define MULTITHREADED_EVENT_HANDLER_HPP

#include <queue>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>
#include <blocks.hpp>
#include <texture.hpp>
#include <render_engine.hpp>

#define GAME_EVENT_NEWSCALE                  0
#define GAME_EVENT_ADDSCALE                  1
#define GAME_EVENT_CAMERA_MOUVEMENT          2
#define GAME_EVENT_SINGLE_CHUNK_POS_REFRESH  3
#define GAME_EVENT_HIGHLIGHT_CHANGE          4
#define GAME_EVENT_SINGLE_CHUNK_MOD          5
#define GAME_EVENT_MULTIPLE_CHUNK_MOD        6
#define GAME_EVENT_INIT_WORLD_RENDER_FLAGS   7

#define STHREAD_OP_BLOCK_ONSCREEN       0b00000001
#define STHREAD_OP_ALL_CHUNK_POS        0b00000010
#define STHREAD_OP_ALL_BLOCK_VISBLE     0b00000100
#define STHREAD_OP_ALL_RENDER_FLAG      0b00001000
#define STHREAD_OP_SINGLE_CHUNK_POS     0b00010000
#define STHREAD_OP_SINGLE_BLOCK_VISBLE  0b00100000
#define STHREAD_OP_SINGLE_RENDER_FLAGS  0b01000000

#define NFS_OP_NONE                     0
#define NFS_OP_ALL_BLOCK_VISIBLE        1
#define NFS_OP_ALL_RENDER_FLAG          2


struct game_event_aditional_data
{
    float scale;
    pixel_coord target;
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
        bool game_is_running;
        SDL_mutex *init_cond;
        SDL_cond  *secondary_frame_op_finish;


        Multithreaded_Event_Handler(Render_Engine&);
        ~Multithreaded_Event_Handler();
        void add_event(game_event&);
        void add_event(const int);
        void add_event(const int, const float);
        void add_event(const int, const pixel_coord);
        void add_event(const int, const chunk_coordonate);
        void add_event(const int, const block_coordonate);
        void add_event(const int, const block_coordonate, Uint16);
        void add_event(const int, const block_coordonate, const block_coordonate);
        void handle();

        /********* NFS OP ************/
        SDL_Thread *NFS_Thread;
        SDL_cond *new_nfs_event;
        void add_nfs_event(const int);
        void drop_all_nfs_event();
        /****************************/
};

int SecondaryThread_operations(void *);

// Non Frame Sensitive Operations
int NFS_operations(void *);

#endif