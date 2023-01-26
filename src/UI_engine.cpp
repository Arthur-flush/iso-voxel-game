#include <game.hpp>

void UI_Engine::render_frame(int game_state, GPU_Target* screen, std::string &New_world_name)
{
    GPU_Clear(UI);

    if(game_state == STATE_MENU)
    {
        New_world_name = "/noworld";

        for(auto i = main_menu.begin(); i != main_menu.end(); i++)
        {
            if((**i).is_mouse_over())
            {
                (**i).change_atlas_id(1);

                New_world_name = (**i).nameid;

                (**i).render(UI);

                (**i).change_atlas_id(0);
            }
            else
            {
                (**i).render(UI);
            }
        }
    }

    if(game_state == STATE_MAIN)
    {


        main_game_hl_type->render(UI);

        // current_block_shader.activate();
        main_game_currentblockpp->render(UI);
        main_game_currentblockp->render(UI);
        main_game_currentblock->render(UI);
        main_game_currentblockn->render(UI);
        main_game_currentblocknn->render(UI);
        // current_block_shader.deactivate();

        main_game_hl_mode->render(UI);
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
    if(game_state == STATE_MENU)
    {
        main_menu.push_back(std::make_unique<UI_tile>(
        "ressources/textures/ui/test/playground.png",
        1, 2, 0,
        screenw, screenh, 
        0.40, 0.10,
        0.25, 0.85));

        main_menu.back()->nameid = "playground";

        main_menu.push_back(std::make_unique<UI_tile>(
        "ressources/textures/ui/test/plain.png",
        1, 2, 0,
        screenw, screenh, 
        0.40, 0.10,
        0.25, 0.70));

        main_menu.back()->nameid = "plain";

        main_menu.push_back(std::make_unique<UI_tile>(
        "ressources/textures/ui/test/island.png",
        1, 2, 0,
        screenw, screenh, 
        0.40, 0.10,
        0.25, 0.55));

        main_menu.back()->nameid = "island";

        main_menu.push_back(std::make_unique<UI_tile>(
        "ressources/textures/ui/test/hut.png",
        1, 2, 0,
        screenw, screenh, 
        0.40, 0.10,
        0.25, 0.40));

        main_menu.back()->nameid = "hut";

    }
    if(game_state == STATE_MAIN)
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
        0.20, 0.20, 
        0.50, 0.90);

        main_game_hl_type = std::make_unique<UI_tile>(
        "ressources/textures/ui/hilight_type.png", 5, 1, 0, 
        screenw, screenh, 
        0.20, 0.20, 
        0.50, 0.10);
        
    }
}