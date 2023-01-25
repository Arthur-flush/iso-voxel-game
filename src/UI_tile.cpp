#include <game.hpp>

UI_tile::UI_tile(const char* filename, int acol, int aline, int aid,
                 int screenw, 
                 int screenh,
                 float sizex,
                 float sizey,
                 float x,
                 float y) 
                 : atlas_col(acol), atlas_line(aline), atlas_id(aid)
{
    if(filename)
        atlas.init_from_file(filename);

    atlas.set_atlas_srcrect(atlas_col, atlas_line, atlas_id);

    GPU_SetAnchor(atlas.ptr, 0.5, 0.5);

    sizex *= screenh;
    sizey *= screenh;

    x *= screenw;
    y *= screenh;

    dest.x = x;
    dest.y = y;

    dest.h = atlas.ptr->h/atlas_line;
    dest.w = atlas.ptr->w/atlas_col;

    scalex = (float)(sizex)/dest.w;
    scaley = (float)(sizey)/dest.h;

    // GPU_SetColor(atlas.ptr, {0, 0, 0, 0});

    // std::cout << filename << "\n";

    // std::cout 
    // << scalex << " "
    // << scaley << " "
    // << dest.x << " "
    // << dest.y << "\n";
}

bool UI_tile::is_mouse_over()
{
    int xlmin = dest.x - dest.w*scalex/2;
    int xlmax = dest.x + dest.w*scalex/2;

    int ylmin = dest.y - dest.h*scaley/2;
    int ylmax = dest.y + dest.h*scaley/2;

    return mouse.x >= xlmin && mouse.x <= xlmax && mouse.y >= ylmin && mouse.y <= ylmax;
}

void UI_tile::change_atlas_id(int newaid)
{
    atlas_id = newaid;
    atlas.set_atlas_srcrect(atlas_col, atlas_line, atlas_id);
}

bool UI_tile::render(GPU_Target *screen)
{
    if(atlas.ptr)
    {
        GPU_BlitScale(atlas.ptr, &atlas.src, screen, dest.x, dest.y, scalex, scaley);
        return is_mouse_over();
    }

    return false;
}