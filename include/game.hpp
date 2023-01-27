#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <memory>
#include <math.h>
#include <chrono>
#include <list>
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#include <UI_engine.hpp>
#include <render_engine.hpp>
#include <multithreaded_event_handler.hpp>

extern pixel_coord mouse;

struct world_extras {
    pixel_coord camera_pos;
    float scale;
    int world_view_position;
};

struct world_extras_select {
    bool camera_pos;
    bool scale;
    bool world_view_position;

    world_extras_select(bool value = false) {
        memset(this, (int)value, sizeof(*this)); // set all to value, cursed level 7/10
    }

    operator bool() { // check if any is true
        world_extras_select tmp(false);
        return memcmp(this, &tmp, sizeof(*this)); 
    }
};

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

        // Loads a world from a savefile and refreshes/initializes everything
        // Argument new_size signals whether or not the projection grid need to be re-allocated,
        // only set it to false if you want fast loading and know what you are doing, true by default
        // if world_extras is not null, it will be filled with the world_extras from the corresponding file
        // if apply_extras is true, the world_extras will be applied to the world
        int load_world(std::string filename, bool new_size = true, bool recenter_camera = false, world_extras* extras = nullptr, world_extras_select extras_select = world_extras_select(true));

        // saves a world extra given as a parameter to a file
        int save_world_extras(std::string filename, world_extras& extras);

        // loads a world extra from a file and fill the given parameter
        int load_world_extras(std::string filename, world_extras* extras);

        // applies a world_extras to the world / Render Engine / whatever else
        void world_extras_apply(world_extras& extras, world_extras_select extras_select = world_extras_select(true));

        // fills a world_extras with the corresponding data
        void world_extras_fill(world_extras& extras);

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