#include <game.hpp>

void UI_Engine::render_frame(int game_state, 
                             GPU_Target* screen, 
                             int cb_id,
                             int bs_line_min,
                             int &new_current_block,
                             std::list<int> &unlocked_blocks,
                             std::string &New_world_name, 
                             std::string &Menu_hl_name)
{
    // GPU_Clear(UI);

    UI = screen;

    // font_menu_shader.activate();

    Menu_hl_name = "/";

    if(game_state == STATE_MAIN_MENU || game_state == STATE_WORLD_SELECTION)
    {
        GPU_SetUniformf(1, timems/1000.0);
        main_menu_title->render(UI, TEXT_MOD_SHADER_ALL);

        main_menu_version->render(UI, TEXT_MOD_HL_PERLETTER | TEXT_MOD_HL_BIGGER);
    }

    if(game_state == STATE_MAIN_MENU)
    {
        for(auto i = main_menu.begin(); i != main_menu.end(); i++)
        {
            if((**i).render(UI, TEXT_MOD_HL_ALL | TEXT_MOD_HL_BIGGER | TEXT_MOD_SHADER_ALL))
            {
                Menu_hl_name = (**i).nameid;
            }
        }
    }


    if(game_state == STATE_WORLD_SELECTION)
    {
        New_world_name = "/noworld";

        for(auto i = world_selection_txt.begin(); i != world_selection_txt.end(); i++)
        {
            if((**i).render(screen, TEXT_MOD_HL_ALL | TEXT_MOD_HL_BIGGER | TEXT_MOD_SHADER_ALL))
            {
                New_world_name = (**i).nameid;
            }
        }
    }
    
    if(game_state == STATE_CONSTRUCTION || game_state == STATE_BLOCK_SELECTION)
    {
        for(int i = 0; i < 8; i++)
        {
            currentblocks[i]->refresh_atlas_id();

            if(cb_id == i)
            {
                // currentblocks[i]->change_size_norm(0.20*screen->h, 0.20*screen->h);
                // currentblocks[i]->render(UI);
                // currentblocks[i]->change_size_norm(0.15*screen->h, 0.15*screen->h);

                currentblocks[i]->change_size_norm(192, 192);
            }
            else
                currentblocks[i]->change_size_norm(128, 128);
            
            currentblocks[i]->render(UI);
        }
    }

    if(game_state == STATE_CONSTRUCTION)
    {
        main_game_hl_type->render(UI, false);

        // SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);
        // main_game_hl_mode->change_position_norm(mouse.x, mouse.y);
        // main_game_hl_mode->render(UI, false);
    }

    if(game_state == STATE_BLOCK_SELECTION)
    {
        GPU_RectangleFilled2(UI, {(float)0.15*screen->w, 
                                  (float)0.15*screen->h, 
                                  (float)0.7*screen->w, 
                                  (float)0.7*screen->h}, 
                                  {0, 0, 0, 165});

        UI_tile block(
        block_atlasx32, 16, 16, 0, 
        screen->w, screen->h, 
        0.10, 0.10,
        0.25, 0.25);

        auto b = unlocked_blocks.begin();
        int i = 0;


        int line = (int)(floor(14*UI_scale));

        if(line < 1)
            line = 1;

        new_current_block = 0;

        for(int j = 0; j/line < bs_line_min && b != unlocked_blocks.end(); b++, j++);

        while(b != unlocked_blocks.end() && (i/line) < 8)
        {
            block.change_atlas_id((*b) - 1);

            // float x = (0.5 + 0.05*(i%line - line/2.0 + 0.5))*screen->w;
            // float y = (0.25 + 0.075*(i/line))*screen->h;

            float x = 0.50*screen->w + 96*(i%line - line/2.0 + 0.5);
            float y = 0.25*screen->h + 96*(i/line);

            block.change_position_norm(x, y);

            // block.change_size_norm(0.10*screen->h, 0.10*screen->h);

            block.change_size_norm(64, 64);

            if(block.is_mouse_over())
            {
                new_current_block = *b;

                // block.change_size_norm(0.15*screen->h, 0.15*screen->h);
                // block.render(UI);
                // block.change_size_norm(0.10*screen->h, 0.10*screen->h);

                block.change_size_norm(192, 192);
            }
            else
                block.change_size_norm(128, 128);
            
            block.render(UI);

            b++;
            i++;
        }
        
    }

    // font_menu_shader.deactivate();

    // GPU_Blit(UI_image, NULL, screen, 0, 0);
}

void UI_Engine::set_ui_current_blocks(int cb_id, int bolck_id)
{
    currentblocks[cb_id]->change_atlas_id(bolck_id-1);
}

void UI_Engine::set_ui_hl_mode(int id)
{
    // main_game_hl_mode->change_atlas_id(id);

    SDL_SetCursor(cursors[id]);
}

void UI_Engine::set_ui_hl_type(int id)
{
    main_game_hl_type->change_atlas_id(id-1);
}

void UI_Engine::generate_tiles(int game_state, int screenw, int screenh)
{
    /*
        filename, atlas_col, atlas_line, atlas default id
        screenw, screenh,
        with onscreen, height onscreen
        centered pos x onscreen, centered pos y onscreen

        ==> tile w/h onscreen are normalized only from the height of the screen
        ==> Positions are normalized from the width and the height of the screen
    */
    
    if(game_state == -1)
    {
        char cur_names[5][256] = 
        {
            "ressources/textures/ui/cursor.png",
            "ressources/textures/ui/cursor_del.png",
            "ressources/textures/ui/cursor_rep.png",
            "ressources/textures/ui/cursor_add.png",
            "ressources/textures/ui/cursor_alt.png"
        };

        for(int i = 0; i < 5; i++)
        {
            GPU_Image *cursor_img;
            SDL_Surface *surface;

            cursor_img = GPU_LoadImage(cur_names[i]);

            surface    =  GPU_CopySurfaceFromImage(cursor_img);
            cursors[i] =  SDL_CreateColorCursor(surface, 32, 32);

            SDL_FreeSurface(surface);
            GPU_FreeImage(cursor_img);
        }


        font_title_shader.load("shader/UI/UI_font.vert", "shader/UI/UI_font_title.frag", NULL);
        font_menu_shader.load("shader/UI/UI_font.vert", "shader/UI/UI_font_menu.frag", NULL);

        font = std::make_shared<UI_tile>(
        "ressources/textures/ui/font/font.png", 26, 3, 0, 
        screenw, screenh, 
        0.20, 0.20,
        0.0, 0.0);

        font_bold = std::make_shared<UI_tile>(
        "ressources/textures/ui/font/font_bold.png", 26, 3, 0, 
        screenw, screenh, 
        0.20, 0.20,
        0.0, 0.0);

        font_light = std::make_shared<UI_tile>(
        "ressources/textures/ui/font/font_light.png", 26, 3, 0, 
        screenw, screenh, 
        0.20, 0.20,
        0.0, 0.0);

        UI_scale = 1.0;

        init_ASCII_to_ATLAS();
    }
    
    if(game_state == STATE_MAIN_MENU)
    {
        float main_menu_y = 0.45;
        float main_menu_yjumps = 0.15;

        main_menu.push_back(std::make_shared<UI_text>(
        font_light,
        TXT_MAIN_MENU_PLAY,
        screenw, screenh,
        0.5, main_menu_y,
        0.08,
        &font_menu_shader,
        IDMENU_PLAY
        ));

        main_menu_y += main_menu_yjumps;
        main_menu.push_back(std::make_shared<UI_text>(
        font_light,
        TXT_MAIN_MENU_QUIT,
        screenw, screenh,
        0.5, main_menu_y,
        0.08,
        &font_menu_shader,
        IDMENU_QUIT   
        ));

        main_menu_version = std::make_unique<UI_text>(
        font_bold,
        VERSION,
        screenw, screenh,
        0.5, 0.95,
        0.040);

        main_menu_title = std::make_unique<UI_text>(
        font_bold,
        TITLE,
        screenw, screenh,
        0.5, 0.15,
        0.08,
        &font_title_shader);
    }
    
    if(game_state == STATE_WORLD_SELECTION)
    {
        std::list<std::string> worlds_names;

        worlds_names.push_back("HUT");
        worlds_names.push_back("ISLAND");
        worlds_names.push_back("PLAIN");
        worlds_names.push_back("PLAYGROUND");
        worlds_names.push_back(".SPRITES/PLAYER");

        float wstxt_y = 0.325;
        float wstxt_yjumps = 0.15;
        float wstxt_x = 0.25;
        float wstxt_size = 0.04;

        for(auto i = worlds_names.begin(); i != worlds_names.end(); i++)
        {
            world_selection_txt.push_back(std::make_shared<UI_text>(
            font,
            *i,
            screenw, screenh,
            wstxt_x, wstxt_y,
            wstxt_size,
            &font_menu_shader,
            *i   
            ));

            wstxt_y += wstxt_yjumps;
        }
    }
    
    if(game_state == STATE_CONSTRUCTION)
    {
        // current_block_shader.load("shader/UI_current_Block.vert", "shader/UI_current_Block.frag", NULL);
        
        block_atlasx32 = std::make_shared<Texture>();

        block_atlasx32->init_from_file("ressources/textures/ui/mosaic 32b.png");

        for(int i = 0; i < 8; i++)
        {
            currentblocks[i] = std::make_unique<UI_tile>(
            block_atlasx32, 16, 16, 0, 
            screenw, screenh, 
            0.15, 0.15,
            0.50 + (3.5-i)*0.075, 0.90);
        }

        main_game_hl_mode = std::make_unique<UI_tile>(
        "ressources/textures/ui/hilight_mode.png", 5, 1, 0, 
        screenw, screenh, 
        0.15, 0.15, 
        0.50, 0.90);

        main_game_hl_type = std::make_unique<UI_tile>(
        "ressources/textures/ui/hilight_type.png", 5, 1, 0, 
        screenw, screenh, 
        0.20, 0.20, 
        0.50, 0.10);
        
    }
}