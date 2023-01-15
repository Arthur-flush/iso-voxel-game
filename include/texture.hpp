#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#include <game.hpp>
#include <constants.hpp>

typedef struct pixel_coord
{
    int x;
    int y;
} pixel_coord;

struct Texture
{
    GPU_Image *ptr;
    GPU_Rect src;
    
    uint32_t id;

    Texture(uint32_t, GPU_Renderer*);
    ~Texture();
};

#endif