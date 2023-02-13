#include <game.hpp>

void UI_Engine::render_frame(int game_state, 
                             GPU_Target* screen, 
                             std::string &New_world_name, 
                             std::string &Menu_hl_name)
{
    GPU_Clear(UI);

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

    if(game_state == STATE_CONSTRUCTION)
    {
        main_game_hl_type->render(UI);

        main_game_currentblockpp->render(UI);
        main_game_currentblockp->render(UI);
        main_game_currentblock->render(UI);
        main_game_currentblockn->render(UI);
        main_game_currentblocknn->render(UI);

        SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);
        main_game_hl_mode->change_position_norm(mouse.x, mouse.y);
        main_game_hl_mode->render(UI);


    }

    if(game_state != STATE_WORLD_SELECTION || New_world_name == "/noworld")
    {
        // SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);
        // GPU_CircleFilled(UI, mouse.x, mouse.y, 7, {62, 62, 62, 255});
        // GPU_CircleFilled(UI, mouse.x, mouse.y, 5, {250, 250, 250, 255});
    }

    GPU_Blit(UI_image, NULL, screen, 0, 0);
}

void UI_Engine::set_ui_current_blocks(std::list<int> &l, std::list<int>::iterator CB)
{
    auto prev = circularPrev(l, CB);
    auto next = circularNext(l, CB);
    main_game_currentblockpp->change_atlas_id(*circularPrev(l, prev)-1);
    main_game_currentblockp->change_atlas_id(*prev-1);
    main_game_currentblock->change_atlas_id(*CB -1);
    main_game_currentblockn->change_atlas_id(*next-1);
    main_game_currentblocknn->change_atlas_id(*circularNext(l, next)-1);
}

void UI_Engine::set_ui_hl_mode(int id)
{
    main_game_hl_mode->change_atlas_id(id);
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

        worlds_names.push_back("hut");
        worlds_names.push_back("island");
        worlds_names.push_back("plain");
        worlds_names.push_back("playground");

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
        main_game_currentblock = std::make_unique<UI_tile>(
        "ressources/textures/ui/mosaic 32b.png", 16, 16, 0, 
        screenw, screenh, 
        0.20, 0.20,
        0.50, 0.90);

        main_game_currentblockp = std::make_unique<UI_tile>(
        "ressources/textures/ui/mosaic 32b.png", 16, 16, 0, 
        screenw, screenh, 
        0.125, 0.125,
        0.575, 0.90);

        main_game_currentblockn = std::make_unique<UI_tile>(
        "ressources/textures/ui/mosaic 32b.png", 16, 16, 0, 
        screenw, screenh, 
        0.125, 0.125,
        0.425, 0.90);

        main_game_currentblocknn = std::make_unique<UI_tile>(
        "ressources/textures/ui/mosaic 32b.png", 16, 16, 0, 
        screenw, screenh, 
        0.0650, 0.0650,
        0.375, 0.90);

        main_game_currentblockpp = std::make_unique<UI_tile>(
        "ressources/textures/ui/mosaic 32b.png", 16, 16, 0, 
        screenw, screenh, 
        0.0650, 0.0650,
        0.625, 0.90);

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