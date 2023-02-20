#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <memory>
#include <math.h>
#include <chrono>
#include <list>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#include <UI_engine.hpp>
#include <render_engine.hpp>
#include <multithreaded_event_handler.hpp>
#include <meteo.hpp>
#include <containers.hpp>

extern pixel_coord mouse;

extern Uint64 timems_start;
extern Uint64 timems;

struct world_extras
{
    pixel_coord camera_pos;
    float scale;
    int world_view_position;
    int meteoid;
};

struct world_extras_select {
    bool camera_pos;
    bool scale;
    bool world_view_position;
    bool meteo;

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
        std::list<int> unlocked_meteo;
        std::list<int>::iterator Current_block;
        std::list<int>::iterator Current_meteo;
        int Current_HL_type;

        FileCircularBuffer undo_buffer;
        FileCircularBuffer redo_buffer;

        const Uint8 max_undo_worlds = 10;

        std::string New_world_name;
        std::string Current_world_name;

        std::string Menu_hl_name;

        bool Show_HUD;

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
        int load_world(std::string filename, 
                       bool load_undo = true,
                       bool new_size = true, 
                       bool recenter_camera = false, 
                       world_extras* extras = nullptr, 
                       world_extras_select extras_select = world_extras_select(false)
                       );

        // saves a world extra given as a parameter to a file
        int save_world_extras(std::string filename, world_extras& extras);

        // loads a world extra from a file and fill the given parameter
        int load_world_extras(std::string filename, world_extras* extras);

        // applies a world_extras to the world / Render Engine / whatever else
        void world_extras_apply(world_extras& extras,
                                world_extras_select extras_select = world_extras_select(true));

        // fills a world_extras with the corresponding data
        void world_extras_fill(world_extras& extras);

        void refresh_world_render();
        void refresh_world_render_fast();
        
        void input_main_menu();
        void input_world_selection();
        void input_block_selection();
        void input_construction();
        void input();

        int undo();
        int redo();

        void save_world_undo(bool clear = true);
        void save_world_redo();

    public :
        Game(GPU_Target*);
        int mainloop();
};

#endif