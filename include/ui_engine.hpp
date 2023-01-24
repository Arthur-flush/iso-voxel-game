#ifndef UI_ENGINE_HPP
#define UI_ENGINE_HPP

#include <iostream>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#include <texture.hpp>
#include <Shader.hpp>
#include <list>

std::list<int>::iterator circularPrev(std::list<int> &, std::list<int>::iterator &);
std::list<int>::iterator circularNext(std::list<int> &, std::list<int>::iterator &);

struct UI_tile_size
{
    float xmin;
    float xmax;

    float ymin;
    float ymax;
};

class UI_tile
{
    private :
        float scalex;
        float scaley;

        int dsizex;
        int dsizey;

        GPU_Rect dest;
        
        int atlas_col;
        int atlas_line;
        int atlas_id;

    public :
        Texture atlas;
        
        UI_tile(const char* filename, 
                int acol, 
                int aline, 
                int aid,
                int screenw,
                int screenh,
                float sizex,
                float sizey,
                float x,
                float y);

        int get_id();

        void change_atlas_id(int newaid);

        bool render(GPU_Target *);

        bool is_mouse_over();

        std::string file_represented;
        std::string nameid;
};

class UI_Engine
{
    std::list<UI_tile> world_selection;

    std::list<std::unique_ptr<UI_tile>> main_menu;

    std::unique_ptr<UI_tile> main_game_currentblockpp;
    std::unique_ptr<UI_tile> main_game_currentblockp;
    std::unique_ptr<UI_tile> main_game_currentblock;
    std::unique_ptr<UI_tile> main_game_currentblockn;
    std::unique_ptr<UI_tile> main_game_currentblocknn;

    std::unique_ptr<UI_tile> main_game_hl_mode;
    std::unique_ptr<UI_tile> main_game_hl_type;

    Shader current_block_shader;

    public :
    
    void set_ui_current_blocks(std::list<int> &, std::list<int>::iterator);
    void set_ui_hl_mode(int);
    void set_ui_hl_type(int);

    void render_frame(int game_state, GPU_Target* screen, std::string &New_world_name);

    void generate_tiles(int game_state, int screenw, int screenh);

    GPU_Image *UI_image;
    GPU_Target *UI;
};

#endif