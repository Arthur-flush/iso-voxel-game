#include <game.hpp>

Texture::Texture()
{
    ptr = NULL;
}

Texture::Texture(uint32_t id)
{
    bool id_verif = true;
    ptr = NULL;

    std::string filename("ressources/textures/");

    switch (id)
    {
    case BLOCK_NORMAL           : filename.append("block/normal.png"); break;

    case BLOCK_AO               : filename.append("block/ao.png"); break;

    case BLOCK_HIGHLIGHT        : filename.append("block/highlight.png"); break;

    case BLOCK_BORDER           : filename.append("block/border.png"); break;

    case BLOCK_PARTS            : filename.append("block/parts.png"); break;

    case MOSAIC                 : filename.append("block/mosaic.png"); break;

    case BACKGROUND_SUNSET      : filename.append("background/sunset.png"); break;

    case SHADERTEXT_WATER       : filename.append("shader/water.png"); break;

    case TEXTURE_UI_DEBUG       : filename.append("block/old/blue.png"); break;

    default:
        id_verif = false;
        break;
    }

    if(id_verif)
    {
        ptr = GPU_LoadImage(filename.c_str());

        src.x = 0;
        src.y = 0;

        src.w = ptr->texture_w;
        src.h = ptr->texture_h;

        GPU_SetImageFilter(ptr, GPU_FILTER_NEAREST);


        if(id == MOSAIC)
            GPU_SetBlending(ptr, false);
        if(id == BACKGROUND_SUNSET)
            GPU_SetBlendMode(ptr, GPU_BLEND_NORMAL);
        if(id == BLOCK_HIGHLIGHT)
            GPU_SetBlendMode(ptr, GPU_BLEND_NORMAL);
        
        // std::cout << filename << "\n" << ptr << "\n";
    }   
};

void Texture::init_from_file(const char* filename)
{
    if(ptr)
        return;

    SDL_RWops* rwops = SDL_RWFromFile(filename, "r");

    ptr = GPU_LoadImage(filename);
    
    // std::cout << "UI " << filename << "\n" << ptr << "\n";

    GPU_SetImageFilter(ptr, GPU_FILTER_NEAREST);

    GPU_SetBlendMode(ptr, GPU_BLEND_NORMAL);
}

void Texture::set_atlas_srcrect(int nbcol, int nbline, int id)
{
    int idcol = id%nbcol;
    int idline = id/nbcol;
    
    src.h = (float)(ptr->h)/nbline;
    src.w = (float)(ptr->w)/nbcol;

    src.x = idcol*src.w;
    src.y = idline*src.h;
}

Texture::~Texture()
{
    // std::cout << "===> Destruction sur " << ptr << "\n";

    if(ptr)
        GPU_FreeImage(ptr);
}