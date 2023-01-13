#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <math.h>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_gpu.h>

#include <render_engine.hpp>
#include <constants.hpp>
#include <blocks.hpp>
#include <texture.hpp>
#include <multithreaded_event_handler.hpp>
#include <projection_grid.hpp>

class Game
{
    private :
        Uint16 state;

        Uint16 Current_block;

        void init(GPU_Target*);
        void input();

        World world;
        Render_Engine RE;
        Multithreaded_Event_Handler GameEvent;

    public :
        Game(GPU_Target*);
        int mainloop();
};

#endif