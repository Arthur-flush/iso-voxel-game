#include <game.hpp>

Uint64 timems = 0;
Uint64 timems_start = 0;

void print_features()
{
    GPU_RendererID test[32];
    GPU_GetRegisteredRendererList(test);

    for(int i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
    {
        std::cout << test[i].name << '\n';
    }
    std::cout << GPU_GetCurrentRenderer()->id.name << '\n';

    std::cout << "vsync : " << GPU_IsFeatureEnabled(GPU_INIT_ENABLE_VSYNC) << "\n";    
    std::cout << "double buff : " << !GPU_IsFeatureEnabled(GPU_INIT_DISABLE_DOUBLE_BUFFER) << "\n";    
    std::cout << "geometry shaders : " << GPU_IsFeatureEnabled(GPU_FEATURE_ALL_SHADERS) << "\n";

}

int main(int argc, char *argv[])
{
    timems_start = Get_time_ms();

    system("cls");

    int statut = EXIT_FAILURE;

    SDL_Init(SDL_INIT_VIDEO);

    // Uint32 GPU_InitFlagEnum = GPU_INIT_DISABLE_VSYNC;
    // GPU_SetPreInitFlags(GPU_InitFlagEnum);

    // SDL_DisplayMode DM;
    // SDL_GetCurrentDisplayMode(0, &DM);

    // GPU_Target *screen =  GPU_InitRenderer(GPU_RENDERER_OPENGL_4, 
    //                                        DM.w,
    //                                        DM.h,
    //                                        GPU_DEFAULT_INIT_FLAGS);

    GPU_Target *screen =  GPU_InitRenderer(GPU_RENDERER_OPENGL_4, 
                                           DEFAULT_WINDOWS_W,
                                           DEFAULT_WINDOWS_H,
                                           GPU_DEFAULT_INIT_FLAGS);

    // GPU_Target *screen =  GPU_InitRenderer(GPU_RENDERER_OPENGL_4, 
    //                                        1600,
    //                                        900,
    //                                        GPU_DEFAULT_INIT_FLAGS);

    // GPU_SetFullscreen(true, false);


    // SDL_ShowCursor(SDL_DISABLE);


    GPU_Image *cursor_img = GPU_LoadImage("ressources/textures/ui/cursor.png");

    if(cursor_img == NULL)
    {
        std::cout << "meeeeuuuuhhh";
    }

    SDL_Surface *surface =  GPU_CopySurfaceFromImage(cursor_img);
    SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 32, 32);
    SDL_SetCursor(cursor);

    GPU_FreeImage(cursor_img);

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);

    GPU_SetRequiredFeatures(0);

    if(screen)
    {
        Game Iso(screen);
        statut = Iso.mainloop();
    }
    
    GPU_Quit();
    SDL_Quit();
    
    std::cout << "Game closed without error.\n";

    return statut;
}