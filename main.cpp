#include <game.hpp>

int main(int argc, char *argv[])
{
    system("cls");

    int statut = EXIT_FAILURE;

    SDL_Init(SDL_INIT_VIDEO);

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);

    GPU_SetRequiredFeatures(0);

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

    // GPU_SetFullscreen(true, false);

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