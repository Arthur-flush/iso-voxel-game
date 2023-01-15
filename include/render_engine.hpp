#ifndef RENDER_ENGINE_HPP
#define RENDER_ENGINE_HPP

#include <memory>

#include <SDL2/SDL.h>
#include <SDL_gpu.h>

#include <coords.hpp>
#include <texture.hpp>
#include <world.hpp>
#include <constants.hpp>
#include <projection_grid.hpp>
// #include <multithreaded_event_handler.hpp>

#include <Shader.hpp>
#include <coords.hpp>

Uint64 Get_time_ms();

class Multithreaded_Event_Handler;

int Thread_refresh_pg_block_visible(void*);

struct Window_Variables
{
        uint16_t FPS_Lock;
        double FrameTime; 
        pixel_coord size;  
        long double scale;
        SDL_Window *ptr;
};

struct Render_Engine
{   
    Render_Engine(World& _world);

    SDL_Thread *SecondaryThread;
    Multithreaded_Event_Handler *GameEvent;

    Window_Variables window;

    GPU_Target *screen;
    GPU_Renderer *renderer;

    World &world;

    pixel_coord target;
    pixel_coord mouse;

    block_coordonate highlight_coord;
    world_coordonate highlight_wcoord;
    
    chunk_coordonate max_render_coord;

    int highlight_mode;

    bool grid_enable;

    Shader shader;
    Shader world_render_shader;
    Shader post_process_shader;
    Shader background_shader;

    GPU_Target *screen2;
    GPU_Image  *final_world_render;

    GPU_Target *screen3;
    GPU_Image  *opaque_world_render;

    GPU_Target *background;
    GPU_Image *background_image;

    std::shared_ptr<Texture> Textures[TEXTURE_MAX_NUMBER] = {NULL};

    long double block_onscreen_size;
    long double block_onscreen_half;
    long double block_onscreen_quarter;
    int max_height_render;

    void refresh_sprite_size();

    void highlight_block();
    
    projection_grid projection_grid;

    bool shader_enable;
    unsigned int shader_features;
    unsigned int sprite_counter;

    bool debug;
    Uint64 timems;

    int current_block_tmp;


    /******** RENDER ***********************/
    void render_grid(); // old

    void render_world();
    bool render_block
         (const chunk_coordonate &, 
          const chunk_coordonate &, 
          GPU_Rect& , GPU_Rect&);

    void render_transparent_world();
    bool render_transparent_block
         (const chunk_coordonate &, 
          const chunk_coordonate &, 
          GPU_Rect& , GPU_Rect&);

    void render_highlighted_blocks();

    void render_frame();
    /***************************************/

    /******** RENDER FLAGS *****************/
    void refresh_all_render_flags2();
    void set_block_renderflags(char, int, int);
    void refresh_block_render_flags
         (const chunk_coordonate&, 
          const int, const int, const int);
    /***************************************/
    
    /******** BLOCK VISIBLE ****************/
    void refresh_pg_block_visible();

    void refresh_all_block_visible2();
    void refresh_line_visible2(int, int, int);
    void refresh_block_visible
         (const chunk_coordonate&, 
          const int, const int, const int);
    /***************************************/

    /******** SHADOWS **********************/
    void set_shadow_context(SDL_Color&, int, int, int);
    void set_block_shadow_context2(int, int, int);

    void refresh_line_shadows(int);
    /***************************************/

    // refresh every block in the progrejection grid
    // when the max_height_render is changed
    void refresh_pg_onscreen();
    void refresh_pg_MHR();

    void rotate_camera(int);

    void highlight_block2();
};


#endif