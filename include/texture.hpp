#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#include <coords.hpp>
#include <world.hpp>

struct Texture
{
    GPU_Image *ptr;
    GPU_Rect src;
    
    uint32_t id;

    Texture(uint32_t);
    ~Texture();
};

#endif