#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <iostream>

#define MAIN_GAME_VERSION "0.7"

//////// WINDOW DEFAULT CONSTANTS ////////
#define DEFAULT_WINDOWS_W 1920
#define DEFAULT_WINDOWS_H 1080
#define DEFAULT_RENDER_DISTANCE 1 // useless at this time

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
#define SHADOW_TOP      128
#define SHADOW_LEFT      64
#define SHADOW_RIGHT     32
#define test 0b00011111

#define HIDE_PART_LEFT    4 // old
#define HIDE_PART_RIGHT   2 // old
#define HIDE_PART_BOTTOM  1 // old

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
#define SHADERTEXT_WATER  0b00000101

//////// BLOCK ID ////////

#define BLOCK_TRANSPARENT_LIMIT 241

#define BLOCK_EMPTY     0
#define BLOCK_DEBUG     1
#define BLOCK_BLUE      2
#define BLOCK_RED       3
#define BLOCK_GREEN     4
#define BLOCK_LIGHT     5
#define BLOCK_SAND      17
#define BLOCK_WATER     241
#endif