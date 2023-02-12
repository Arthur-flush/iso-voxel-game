// #include <constants.hpp>
// #include <sprites.hpp>
#include <game.hpp>

void sprite::init_sprite(Multithreaded_Event_Handler *_GameEvent, 
                         World *_world, 
                         Render_Engine *_RE,
                         std::string& name)
{
    world = _world;
    RE = _RE;
    GameEvent = _GameEvent;

    pos = {-1, -1, -1};

    std::string total_filename = "saves/.sprites/";
    total_filename.append(name);
    total_filename.append("/world.isosave");

    frames.load_from_file(total_filename.c_str());

    set_frame(0, 0);
}

void sprite::set_frame(int x, int y)
{
    int basex = x*SPRITES_WIDTH;
    int basey = y*SPRITES_WIDTH;
    int basez = 8;

    for(int x = 0; x < SPRITES_WIDTH; x ++)
    for(int y = 0; y < SPRITES_WIDTH; y ++)
    for(int z = 0; z < SPRITES_HEIGHT; z ++)
    {
        voxels[x][y][z].id = frames.get_block_id_wcoord(basex+x, basey+y, basez+z);
    }
}

void sprite::remove()
{
    sprite_voxel *v = &voxels[0][0][0];

    // Deleting sprite instance frome the world & setup voxels next coords
    for(int x = 0; x < SPRITES_WIDTH;  x ++)
    for(int y = 0; y < SPRITES_WIDTH;  y ++)
    for(int z = 0; z < SPRITES_HEIGHT; z ++)
    {
        v = &voxels[x][y][z];

        if(!v->is_occluded)
        {
            block_coordonate bc = world->convert_wcoord(v->wcoord.x, 
                                                        v->wcoord.y, 
                                                        v->wcoord.z);
            
            block *b = &world->
                        chunks
                        [bc.chunk.x]
                        [bc.chunk.y]
                        [bc.chunk.z]
                        .blocks
                        [bc.x]
                        [bc.y]
                        [bc.z];

            b->id = BLOCK_EMPTY;
            // world->compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);
        }
    }

    compress_chunks();
}

bool sprite::tp(fcoord3D fcoord)
{
    coord3D coord;
    subvoxel_pos = fcoord;

    coord.x = floor(fcoord.x);
    coord.y = floor(fcoord.y);
    coord.z = floor(fcoord.z);

    if(coord.x == pos.x && coord.y == pos.y && coord.z == pos.z)
        return false;

    if(pos.x != -1)
    {
        remove();
        refresh_display();
    }

    set_in_interval(coord.x, SPRITES_WIDTH/2, world->max_block_coord.x-SPRITES_WIDTH/2);
    set_in_interval(coord.y, SPRITES_WIDTH/2, world->max_block_coord.y-SPRITES_WIDTH/2);
    set_in_interval(coord.z, 0, world->max_block_coord.z-SPRITES_HEIGHT);
    pos = coord;

    sprite_voxel *v = &voxels[0][0][0];

    // Building sprite in the world

    // rotation = 3;

    for(int x = 0; x < SPRITES_WIDTH;  x ++)
    for(int y = 0; y < SPRITES_WIDTH;  y ++)
    for(int z = 0; z < SPRITES_HEIGHT; z ++)
    {
        v = &voxels[x][y][z];

        // if(!v->id) continue;

        v->is_occluded = true;

        // int x2 = SPRITES_WIDTH-1-x;
        int x2 = x;
        int y2 = y;
        int z2 = z;

        switch (rotation)
        {
        case 1 :
            x2 = y;
            y2 = x;
            y2 = SPRITES_WIDTH-1-y2;
            break;

        case 2 :
            x2 = y;
            y2 = x;
            x2 = SPRITES_WIDTH-1-x2;
            break;

        case 3 :
            y2 = SPRITES_WIDTH-1-y;
            x2 = SPRITES_WIDTH-1-x;
        default:
            break;
        }


        v->wcoord = {coord.x - SPRITES_WIDTH/2 + x2, coord.y - SPRITES_WIDTH/2 + y2, coord.z + z2};

        block_coordonate bc = world->convert_wcoord(v->wcoord.x, 
                                                    v->wcoord.y, 
                                                    v->wcoord.z);
        
        block *b = &world->
                    chunks
                    [bc.chunk.x]
                    [bc.chunk.y]
                    [bc.chunk.z]
                    .blocks
                    [bc.x]
                    [bc.y]
                    [bc.z];

        if(!b->id)
        {
            v->is_occluded = false;
            b->id = v->id;

            // world->compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);
        }
    }

    compress_chunks();

    refresh_display();
    RE->projection_grid.refresh_all_identical_line();

    return true;
}

bool sprite::move(fcoord3D vel)
{
    if(pos.x == -1)
    {
        return false;
    }

    if(vel.y < 0.f)
        rotation = 3;
    
    else
    if(vel.y > 0.f)
        rotation = 0; 

    else if(vel.x < 0)
        rotation = 2;
    
    else if(vel.x > 0)
        rotation = 1;

    fcoord3D newpos;

    newpos.x = subvoxel_pos.x + vel.x;
    newpos.y = subvoxel_pos.y + vel.y;
    newpos.z = subvoxel_pos.z + vel.z;

    // include some valid newpos verif here

    return tp(newpos);
}

void sprite::compress_chunks()
{
    block_coordonate wcoo = world->convert_wcoord(pos.x, pos.y, pos.z);

    int xbeg = wcoo.chunk.x >= 2 ? wcoo.chunk.x-2 : 0; 
    int ybeg = wcoo.chunk.y >= 2 ? wcoo.chunk.y-2 : 0; 
    int zbeg = wcoo.chunk.z;

    int xend = wcoo.chunk.x > world->max_chunk_coord.x-2 ? world->max_chunk_coord.x : wcoo.chunk.x+2;
    int yend = wcoo.chunk.y > world->max_chunk_coord.y-2 ? world->max_chunk_coord.y : wcoo.chunk.y+2;
    int zend = wcoo.chunk.z > world->max_chunk_coord.z-4 ? world->max_chunk_coord.z : wcoo.chunk.z+4;

    for(int x = xbeg; x <= xend; x++)
    for(int y = ybeg; y <= yend; y++)
    for(int z = zbeg; z <= zend; z++)
    {
        world->compress_chunk(x, y, z);
    }
}

void sprite::refresh_display()
{
    world_coordonate wcoo = pos;
    
    world->invert_wvp(wcoo.x, wcoo.y);

    int xbeg = wcoo.x - SPRITES_WIDTH/2 - 1;
    int ybeg = wcoo.y - SPRITES_WIDTH/2 - 1;
    int zbeg = wcoo.z - 1;

    int xend = wcoo.x + SPRITES_WIDTH/2 + 1;
    int yend = wcoo.y + SPRITES_WIDTH/2 + 1;
    int zend = wcoo.z + SPRITES_HEIGHT + 1;

    int y = yend;
    int x, z;

    for(x = xbeg; x <= xend; x++)
    for(z = zbeg; z <= zend; z++)
    {
        int shiftx = world->max_block_coord.x-x-1;
        int shifty = world->max_block_coord.y-y-1;
        int shiftz = RE->max_height_render-z-1;

        int shiftmin = shiftx < shifty ? shiftx : shifty;
        shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

        int wx, wy, wz;

        wx = x + shiftmin;
        wy = y + shiftmin;
        wz = z + shiftmin;

        RE->refresh_line_visible2(wx, wy, wz);

        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);

        RE->set_block_renderflags(pgc.x, pgc.y, pgc.z);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }

    for(y = ybeg; y <= yend; y++)
    for(z = zbeg; z <= zend; z++)
    {
        int shiftx = world->max_block_coord.x-x-1;
        int shifty = world->max_block_coord.y-y-1;
        int shiftz = RE->max_height_render-z-1;

        int shiftmin = shiftx < shifty ? shiftx : shifty;
        shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

        int wx, wy, wz;

        wx = x + shiftmin;
        wy = y + shiftmin;
        wz = z + shiftmin;

        RE->refresh_line_visible2(wx, wy, wz);

        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);

        RE->set_block_renderflags(pgc.x, pgc.y, pgc.z);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }

    for(x = xbeg; x <= xend+1; x++)
    for(y = ybeg; y <= yend; y++)
    {
        int shiftx = world->max_block_coord.x-x-1;
        int shifty = world->max_block_coord.y-y-1;
        int shiftz = RE->max_height_render-z-1;

        int shiftmin = shiftx < shifty ? shiftx : shifty;
        shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

        int wx, wy, wz;

        wx = x + shiftmin;
        wy = y + shiftmin;
        wz = z + shiftmin;

        RE->refresh_line_visible2(wx, wy, wz);

        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);

        RE->set_block_renderflags(pgc.x, pgc.y, pgc.z);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }

    world_coordonate wcoo_shadow = pos;
    wcoo_shadow.x -= pos.z + SPRITES_HEIGHT + 1;

    world->invert_wvp(wcoo_shadow.x, wcoo_shadow.y);
    set_in_interval(wcoo_shadow.x, 0, world->max_block_coord.x);
    set_in_interval(wcoo_shadow.y, 0, world->max_block_coord.y);

    for(x = xend; x <= wcoo_shadow.x; x++)
    for(y = ybeg; y <= yend; y++)
    for(z = zbeg; z <= zend; z++)
    {
        int wx = x, wy = y, wz = z;
        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }
}