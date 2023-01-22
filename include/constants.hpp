#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <iostream>
#include <SDL2/SDL.h>

#define MAIN_GAME_VERSION "0.9.0"

//////// WINDOW DEFAULT CONSTANTS ////////
#define DEFAULT_WINDOWS_W 1920
#define DEFAULT_WINDOWS_H 1080

//////// SCALE ////////
/*
Texture real size : 256px x 256px
Scale must be integrer or = (1/2)^(-n) to mitigate render bugs
tested floats exemple :
0.5
0.125
0.625
0.03125
0.015625
*/
#define DEFAULT_SCALE              (long double)0.125
#define MAX_FLOAT_SCALE            (long double)8
#define MIN_FLOAT_SCALE            (long double)0.0078125
#define PANORAMA_SCALE_THRESHOLD   (long double)0.0625

//////// GAME STATE ////////
#define STATE_QUIT 0
#define STATE_MAIN 1

//////// CHUNKS ////////
#define CHUNK_SIZE 8

//////// SAVE/LOAD ERROR CODES ////////
#define SAVE_ERROR_NONE 0
#define SAVE_ERROR_FILE_NOT_OPEN 1

//////// HIGHLIGHT ////////
#define HIGHLIGHT_NONE        0
#define HIGHLIGHT_REMOVE      1
#define HIGHLIGHT_PLACE       2
#define HIGHLIGHT_PLACE_ALT   3
#define HIGHLIGHT_DEBUG       4

//////// SHADER FEATURES ////////
#define SFEATURE_GLOBAL_ILLUMINATION 1
#define SFEATURE_AMBIANT_OCCLUSION   2
#define SFEATURE_BLOCK_BORDERS       4
#define SFEATURE_SHADOWS             8
#define SFEATURE_BLOOM               16

//////// RENDER FLAGS ////////
#define SHADOW_TOP      128
#define SHADOW_LEFT      64
#define SHADOW_RIGHT     32

//////// TEXTURES ////////
#define BLOCK_TEXTURE_SIZE    256
#define MOSAIC_TEXTURE_SIZE   4096
#define MOSAIC_TOTAL_BLOCK    256
#define MOSAIC_BLOCK_PER_LINE 16
#define MOSAIC_BPL_LOG        4
#define TEXTURE_MAX_NUMBER    0b11111111
#define TEXTURE_MIN_ID        0b00000000
#define TEXTURE_BLOCK_ID      0b00000000
#define TEXTURE_BACKGROUND_ID 0b11000000

#define BACKGROUND_SUNSET 0b11000000
#define MOSAIC            0b00000001
#define BLOCK_NORMAL      0b00000010
#define BLOCK_AO          0b00000011
#define BLOCK_HIGHLIGHT   0b00000100
#define BLOCK_BORDER      0b00000101
#define SHADERTEXT_WATER  0b00000110

//////// CHUNK COMPRESSION ////////
#define CHUNK_EMPTY       0b000000000
#define CHUNK_NON_UNIFORM 0b100000000

//////// BLOCK ID ////////
#define BLOCK_TRANSPARENT_LIMIT     241
#define BLOCK_EMPTY                 0
#define BLOCK_DEBUG                 1
#define BLOCK_BLUE                  2
#define BLOCK_RED                   3
#define BLOCK_GREEN                 4
#define BLOCK_LIGHT                 5
#define BLOCK_SAND                  17
#define BLOCK_WATER                 241
#define BLOCK_GLASS_COLORLESS       242
#define BLOCK_GLASS_RED             243
#define BLOCK_GLASS_BLUE            244
#define BLOCK_GLASS_GREEN           245
#define BLOCK_GLASS_YELLOW          246
#define BLOCK_GLASS_CYAN            247
#define BLOCK_GLASS_MAGENTA         248

const Uint8 BLOCK_PALETTE[] = {
    BLOCK_RED,
    BLOCK_DEBUG,
    BLOCK_BLUE,
    BLOCK_GREEN,
    BLOCK_SAND,
    BLOCK_GLASS_COLORLESS,
    BLOCK_GLASS_RED,
    BLOCK_GLASS_BLUE,
    BLOCK_GLASS_GREEN,
    BLOCK_GLASS_YELLOW,
    BLOCK_GLASS_CYAN,
    BLOCK_GLASS_MAGENTA,
    BLOCK_WATER
};


//////// PHYSICS EVENT IDs ////////
#define PHYSICS_EVENT_WATER 0

#endif