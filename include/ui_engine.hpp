#ifndef UI_ENGINE_HPP
#define UI_ENGINE_HPP

#include <iostream>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#include <constants.hpp>
#include <texture.hpp>
#include <Shader.hpp>
#include <list>
#include <vector>

#define TEXT_MOD_SHADER_ALL    1
#define TEXT_MOD_SHADER_HL     2
#define TEXT_MOD_HL_BIGGER     32
#define TEXT_MOD_HL_PERLETTER  4
#define TEXT_MOD_HL_ALL        8
#define TEXT_MOD_GET_ID        16

std::list<int>::iterator circularPrev(std::list<int> &, std::list<int>::iterator &);
std::list<int>::iterator circularNext(std::list<int> &, std::list<int>::iterator &);

void init_ASCII_to_ATLAS();

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

        // int get_id();

        void change_atlas_id(int newaid);
        void change_size_norm(float sizex, float sizey);
        void change_position_norm(float x, float y);

        void render(GPU_Target *);

        bool is_mouse_over();


        std::string file_represented;
        std::string nameid;
};

class UI_text
{
    private :
        float centered_posx;
        float centered_posy;

        float charwith;

        std::shared_ptr<UI_tile> font;
        std::vector<Uint8> text;

        Shader *shader;

    public :
        UI_text(
            std::shared_ptr<UI_tile> textfont,
            std::string const &strtext,
            int screenw,
            int screenh,
            float centered_x,
            float centered_y,
            float charsize,
            Shader *textshdr = nullptr,
            const std::string &textnameid = IDMENU_UNTITLED
            );

        bool render(GPU_Target *, Uint64);

        std::string nameid;
};

class UI_Engine
{
    std::list<std::shared_ptr<UI_tile>> world_selection;
    std::list<std::shared_ptr<UI_text>> world_selection_txt;
    std::list<std::shared_ptr<UI_text>> main_menu;

    std::unique_ptr<UI_text> main_menu_version;
    std::unique_ptr<UI_text> main_menu_title;

    std::unique_ptr<UI_tile> main_game_currentblockpp;
    std::unique_ptr<UI_tile> main_game_currentblockp;
    std::unique_ptr<UI_tile> main_game_currentblock;
    std::unique_ptr<UI_tile> main_game_currentblockn;
    std::unique_ptr<UI_tile> main_game_currentblocknn;

    std::unique_ptr<UI_tile> main_game_hl_mode;
    std::unique_ptr<UI_tile> main_game_hl_type;

    Shader font_title_shader;
    Shader font_menu_shader;

    std::shared_ptr<UI_tile> font;
    std::shared_ptr<UI_tile> font_bold;
    std::shared_ptr<UI_tile> font_light;

    public :
    
    void set_ui_current_blocks(std::list<int> &, std::list<int>::iterator);
    void set_ui_hl_mode(int);
    void set_ui_hl_type(int);

    void render_frame(int game_state, 
                      GPU_Target* screen, 
                      std::string &New_world_name, 
                      std::string &Menu_hl_name);

    void generate_tiles(int game_state, int screenw, int screenh);

    GPU_Image *UI_image;
    GPU_Target *UI;
};

#endif