#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <memory>
#include <math.h>
#include <chrono>
#include <list>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#include <ui_engine.hpp>
#include <render_engine.hpp>
#include <multithreaded_event_handler.hpp>

extern pixel_coord mouse;

class Game
{
    private :
        Uint16 state;

        // Uint16 Current_block;
        std::list<int> unlocked_blocks;
        std::list<int>::iterator Current_block;

        std::string New_world_name;
        std::string Current_world_name;

        World world;
        UI_Engine UI;
        Render_Engine RE;
        Multithreaded_Event_Handler GameEvent;

        void init(GPU_Target*);
        void init_Render_Engine(GPU_Target*);

        void generate_debug_world();

        // Load a world from a savefile and refresh/initialize everything
        // Argument new_size tell if the projection grid need to be re-allocated,
        // only set-it to false if you want a fast loading and know what you are doing, true by default
        int load_world(std::string filename, bool new_size = true, bool recenter_camera = false);

        void refresh_world_render();
        void refresh_world_render_fast();
        
        void input_mainmenu();
        void input_maingame();
        void input();



    public :
        Game(GPU_Target*);
        int mainloop();
};

#endif