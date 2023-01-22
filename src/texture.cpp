#include <texture.hpp>

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

    case MOSAIC                 : filename.append("block/mosaic.png"); break;

    case BACKGROUND_SUNSET      : filename.append("background/sunset.png"); break;

    case SHADERTEXT_WATER       : filename.append("shader/water.png"); break;

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

        //GPU_BlendPresetEnum {
        //GPU_BLEND_NORMAL = 0, GPU_BLEND_PREMULTIPLIED_ALPHA = 1, GPU_BLEND_MULTIPLY = 2, GPU_BLEND_ADD = 3,
        //GPU_BLEND_SUBTRACT = 4, GPU_BLEND_MOD_ALPHA = 5, GPU_BLEND_SET_ALPHA = 6, GPU_BLEND_SET = 7,
        //GPU_BLEND_NORMAL_KEEP_ALPHA = 8, GPU_BLEND_NORMAL_ADD_ALPHA = 9, GPU_BLEND_NORMAL_FACTOR_ALPHA = 10

        if(id == MOSAIC)
            GPU_SetBlending(ptr, false);
            // GPU_SetBlendMode(ptr, GPU_BLEND_NORMAL);
            // GPU_SetBlendMode(ptr, GPU_BLEND_PREMULTIPLIED_ALPHA);
        if(id == BACKGROUND_SUNSET)
            GPU_SetBlendMode(ptr, GPU_BLEND_NORMAL);
        if(id == BLOCK_HIGHLIGHT)
            GPU_SetBlendMode(ptr, GPU_BLEND_NORMAL);
    }   
};

Texture::~Texture()
{
    if(ptr)
        GPU_FreeImage(ptr);
}