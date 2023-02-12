#ifndef SPRITES_HPP
#define SPRITES_HPP

#include <world.hpp>
#include <coords.hpp>
#include <multithreaded_event_handler.hpp>

#define SPRITES_WIDTH 16
#define SPRITES_HEIGHT 24

extern Uint64 timems;

struct sprite_voxel
{
    int id;
    bool is_occluded;
    coord3D wcoord;
    // coord3D scoord;
};

struct animation_frame
{
    coord2D pos;
    Uint64  beg;
    Uint64  end;
};

class animation_loop
{
    private :
        Uint64 duration; //(ms)

        int nbframes;
        animation_frame *frames;

    public :

        animation_loop();
        ~animation_loop();

        void init(int _nbframes);
        void addframe(animation_frame new_animframe);
};

class sprite
{
    private :
        sprite_voxel voxels[SPRITES_WIDTH][SPRITES_WIDTH][SPRITES_HEIGHT];
        fcoord3D subvoxel_pos;
        fcoord3D velocity;

        World *world;
        Render_Engine *RE;
        Multithreaded_Event_Handler *GameEvent;

        World frames;

        coord3D pos;
        void compress_chunks();

        void set_frame(int x, int y);

        int rotation;

    public :

        void init_sprite(Multithreaded_Event_Handler *_GameEvent, 
                         World *_world, 
                         Render_Engine *_RE,
                         std::string& name);

        bool move(fcoord3D vel);
        bool tp(fcoord3D fcoord);

        void update();
        void remove();
        void refresh_display();
};


#endif