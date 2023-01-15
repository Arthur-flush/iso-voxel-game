#ifndef COORDS_HPP
#define COORDS_HPP

struct coord3D
{
    int x;
    int y;
    int z;
};

typedef coord3D chunk_coordonate;

typedef coord3D world_coordonate;

struct block_coordonate : coord3D
{
    chunk_coordonate chunk;
};

struct coord2D
{
    int x;
    int y;
};

typedef coord2D pixel_coord;

struct fcoord3D
{
    float x;
    float y;
    float z;
};

struct fcoord2D
{
    float x;
    float y;
};


#endif