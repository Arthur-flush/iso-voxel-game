#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL2/SDL.h>
#include <SDL_gpu.h>

#include <game.hpp>
#include <constants.hpp>
#include <coords.hpp>

struct Texture
{
    GPU_Image *ptr;
    GPU_Rect src;
    
    uint32_t id;

    Texture(uint32_t, GPU_Renderer*);
    ~Texture();
};

#endif