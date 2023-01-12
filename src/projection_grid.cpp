#include <game.hpp>

projection_grid::projection_grid()
{
    pos[0] = NULL;
    pos[1] = NULL;
    pos[2] = NULL;
}

projection_grid::~projection_grid()
{
    // std::cout << "Calling projection_grid destructor\n";

    for(int face = 0; face < 3; face ++)
    {
        if(pos[face])
        {
            for(int i = 0; i < size[face][0]; i++)
                if(pos[face][i])
                    delete [] pos[face][i];

            delete [] pos[face];
        }
    }
}

void projection_grid::init_pos(const int sizex, const int sizey, const int sizez)
{
    if(pos[0] || pos[1] || pos[2])
    {
        std::cerr << "\nRENDER FATAL ERROR : Can't initialize non empty projection grid.";
        return;
    }
    
    size[0][0] = sizey+1;
    size[0][1] = sizez+1;

    size[1][0] = sizex+1;
    size[1][1] = sizez+1;

    size[2][0] = sizex+1;
    size[2][1] = sizey+1;

    for(int face = 0; face < 3; face ++)
    {
        pos[face] = new screen_block*[size[face][0]];

        for(int i = 0; i < size[face][0]; i++)
        {
            pos[face][i] = new screen_block[size[face][1]];

            for(int j = 0; j < size[face][1]; j++)
            {
                pos[face][i][j].height = 0xabab; // debug
                pos[face][i][j].is_on_screen = true;
                pos[face][i][j].render_flags = {0, 0, 0, 0};
                pos[face][i][j].render_flags_transparent = {0, 0, 0, 0};
                pos[face][i][j].block = NULL;
                pos[face][i][j].transparent_block = NULL;
                pos[face][i][j].identical_line_counter = 0;
                pos[face][i][j].identical_line_counter_transparent = 0;
            }
        }
    }

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 2; j++)
        {
            visible_frags[i][j].beg = 0;
            visible_frags[i][j].end = size[i][j];
        }
}

screen_block* projection_grid::get_pos(Uint8 face, Uint32 i, Uint32 j)
{
    if(face > 2 || i >= (Uint32)size[face][0] || j >= (Uint32)size[face][1])
        return NULL;
    
    return &pos[face][i][j];
}

screen_block* projection_grid::get_pos(chunk_coordonate coord, int x, int y, int z)
{
    return get_pos_world(coord.x*CHUNK_SIZE+x, coord.y*CHUNK_SIZE+y, coord.z*CHUNK_SIZE+z);
}

screen_block* projection_grid::get_pos_world(int x, int y, int z)
{

    int shift = x < y ? x : y;
    shift = shift < z ? shift : z;

    x -= shift;
    y -= shift;
    z -= shift;
    
    if(x < 0 || y < 0 || z < 0 || x >= size[1][0] || y >= size[0][0] || z >= size[0][1])
        return NULL;
        
    if(x == 0)
    {
        return &pos[0][y][z];
    }
    if(y == 0)
    {
        return &pos[1][x][z];
    }
    if(z == 0)
    {
        return &pos[2][x][y];
    }

    return NULL;
}

chunk_coordonate projection_grid::convert_wcoord(int x, int y, int z)
{

    int shift = x < y ? x : y;
    shift = shift < z ? shift : z;

    x -= shift;
    y -= shift;
    z -= shift;
    
    if(x < 0 || y < 0 || z < 0 || x >= size[1][0] || y >= size[0][0] || z >= size[0][1])
        return (chunk_coordonate){-1, -1, -1};
    
    if(x == 0)
    {
        return (chunk_coordonate){0, y, z};
    }
    if(y == 0)
    {
        return (chunk_coordonate){1, x, z};
    }
    if(z == 0)
    {
        return (chunk_coordonate){2, x, y};
    }

    return {-1, -1, -1};
}

void set_in_interval(int& x, const int min, const int max)
{
    if(x < min)
        x = min;
    else if(x > max)
        x = max;
}

void projection_grid::refresh_visible_frags(pixel_coord t, Uint16 Rx, Uint16 Ry, long double b)
{
    visible_frags[0][0].beg = floor((2.0*(t.x - Rx))/b -1);
    visible_frags[0][0].end = floor((2.0*t.x)/b + 1)+1;

    set_in_interval(visible_frags[0][0].end, 0, size[0][0]-1);

    visible_frags[0][1].beg = floor(2.0*(t.y - Ry)/b);
    visible_frags[0][1].end = floor((2.0*t.y)/b + 1 + visible_frags[0][0].end/2);

    visible_frags[1][0].beg = floor((-2.0*t.x)/b - 1);
    visible_frags[1][0].end = floor((2.0*(Rx-t.x))/b + 1)+1;

    set_in_interval(visible_frags[1][0].end, 0, size[1][0]-1);

    visible_frags[1][1].beg = floor(2.0*(t.y - Ry)/b);
    visible_frags[1][1].end = floor((2.0*t.y)/b + 1 + visible_frags[1][0].end/2)-1;

    const int x = 0;
    const int y = 1;

    visible_frags[2][x].beg = floor((-2*t.y-t.x)/b-1.5)+1;
    visible_frags[2][x].end = floor((Rx - t.x + 2*(Ry-t.y))/b + 1.5)+1;

    visible_frags[2][y].beg = floor((t.x - 2*t.y - Rx)/b - 1.5)+1;
    visible_frags[2][y].end = floor((t.x - 2*t.y + 2*Ry)/b + 1.5)+1;

    /*
    //// TENTAIVE 2 //////////
    // 0, y, z
    // visible_frags[0][0].beg = floor((2.0*(t.x - Rx))/b -1);
    // visible_frags[0][0].end = floor((2.0*t.x)/b + 1);

    // visible_frags[0][1].beg = floor(2.0*(t.y - Ry)/b + 1);
    // visible_frags[0][1].end = floor((2.0*t.y)/b + 1 + size[0][1]/2);
    //////////////////////////////////////////////////////////////////

    ////// x, 0, z
    // visible_frags[1][0].beg = floor((-2.0*t.x)/b - 1);
    // visible_frags[1][0].end = floor((2.0*(Rx-t.x))/b + 1);

    // visible_frags[1][1].beg = floor(2.0*(t.y - Ry)/b + 1);
    // visible_frags[1][1].end = floor((2.0*t.y)/b + 1 + size[1][1]/2);
    //////////////////////////////////////////////////////////////////

    // x, y, 0
    // visible_frags[2][0].end = floor(4.0*(Ry - t.y)/b + 2);

    // const int x = 0;
    // const int y = 1;

    // visible_frags[2][x].beg = floor(-4.0*(t.y)/b - 2 - size[2][1]);
    // set_in_interval(visible_frags[2][x].beg, 0, size[2][x]);

    // visible_frags[2][y].end = floor(4.0*(Ry - t.y)/b + 2);

    // float ymin1 = floor(-2.0*(Rx - t.x)/b - 1);;
    // float ymin2 = floor(visible_frags[2][x].beg);
    // visible_frags[2][y].beg = ymin1 > ymin2 ? ymin1 : ymin2;

    // set_in_interval(visible_frags[2][y].end, 0, size[2][y]);

    // float xmax1 = floor(4.0*(Ry - t.y)/b + 2);
    // float xmax2 = floor(2.0*(Rx - t.x)/b + 1 + visible_frags[2][y].end);
    // visible_frags[2][x].end = xmax1 < xmax2 ? xmax1 : xmax2;





    ///// TENTAIVE 1 //////////
    // visible_frags[2][0].end = xmax2;

    // float nRy = Ry*(3.0/4.0);
    // float nRx = Rx*(3.0/1.0);
    // float xmax3 = floor((2.0*(nRy-t.y) + nRx - t.x)/b + 1.5 + size[2][1]/2);
    // visible_frags[2][0].end = visible_frags[2][0].end  < xmax3 ? visible_frags[2][0].end : xmax3;
    // visible_frags[2][0].end = floor((2.0*(nRy-t.y) + nRx - t.x)/b + 1.5 + size[2][1]/2);

    // long double dec  = (Rx+2.0*Ry)/b + 3;

    // visible_frags[2][0].beg = floor((t.x-2.0*t.y)/b + 1.5);
    // visible_frags[2][0].end = floor(visible_frags[2][0].beg + dec);

    // visible_frags[2][1].beg = floor((t.x - 2*t.y - Rx)/b - 1.5);
    // visible_frags[2][1].end = floor(visible_frags[2][1].beg + dec);

    */

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 2; j++)
        {
            set_in_interval(visible_frags[i][j].end, 0, size[i][j]-1);
            set_in_interval(visible_frags[i][j].beg, 0, size[i][j]-1);
        }
    
}

void projection_grid::refresh_all_identical_line()
{
    // std::cout << "PG : refreshing all identical line...";

    int identical_line_counter = 0;
    int identical_line_counter_transparent = 0;

    SDL_Color *rf = {0};
    SDL_Color *rf2 = {0};

    Uint8 id = 0;
    Uint8 id2 = 0;

    SDL_Color *trf = {0};
    SDL_Color *trf2 = {0};

    Uint8 tid = 0;
    Uint8 tid2 = 0;

    screen_block *sb;

    // x y
    int face = 2;

    for(int i = 0; i < size[face][0]; i++)
    {
        identical_line_counter = 0;
        identical_line_counter_transparent = 0;

        rf2 = &pos[face][i][size[face][1]-2].render_flags;
        id2 = pos[face][i][size[face][1]-2].block ? pos[face][i][size[face][1]-2].block->id : 0;

        trf2 = &pos[face][i][size[face][1]-2].render_flags_transparent;
        tid2 = pos[face][i][size[face][1]-2].transparent_block ? pos[face][i][size[face][1]-2].transparent_block->id : 0;  

        for(int j = size[face][1]-3; j >= 0; j--)
        {
            sb = &pos[face][i][j];

            rf = &sb->render_flags;
            id = sb->block ? sb->block->id : 0;


            if(identical_line_counter < IDENDICAL_LINE_MAX && 
               rf->r == rf2->r &&
               rf->g == rf2->g && 
               rf->b == rf2->b && 
               rf->a == rf2->a &&
               id == id2)
                identical_line_counter ++;
            else
                identical_line_counter = 0;

            rf2 = rf;
            id2 = id;

            trf = &sb->render_flags_transparent;
            tid = sb->transparent_block ? sb->transparent_block->id : 0;

            if(identical_line_counter_transparent < IDENDICAL_LINE_MAX && 
               trf->r == trf2->r &&
               trf->g == trf2->g && 
               trf->b == trf2->b && 
               trf->a == trf2->a &&
               tid == tid2)
                identical_line_counter_transparent ++;
            else
                identical_line_counter_transparent = 0;

            sb->identical_line_counter_transparent = identical_line_counter_transparent;
            sb->identical_line_counter = identical_line_counter;

            trf2 = trf;
            tid2 = tid;
        }
    }


    // z x
    face = 1;

    for(int i = 0; i < size[face][1]; i++)
    {
        identical_line_counter = 0;

        rf2 = &pos[face][size[face][1]-1][i].render_flags;
        id2 = pos[face][size[face][1]-1][i].block ? pos[face][size[face][1]-1][i].block->id : 0;

        for(int j = size[face][0]-2; j >= 0; j--)
        {
            sb = &pos[face][j][i];

            rf = &sb->render_flags;
            id = sb->block ? sb->block->id : 0;

            if(identical_line_counter < IDENDICAL_LINE_MAX && 
               rf->r == rf2->r &&
               rf->g == rf2->g && 
               rf->b == rf2->b && 
               rf->a == rf2->a &&
               id == id2)
                identical_line_counter ++;
            else
                identical_line_counter = 0;

            sb->identical_line_counter = identical_line_counter;

            rf2 = rf;
            id2 = id;

            rf2 = rf;
            id2 = id;

            trf = &sb->render_flags_transparent;
            tid = sb->transparent_block ? sb->transparent_block->id : 0;

            if(identical_line_counter_transparent < IDENDICAL_LINE_MAX && 
               trf->r == trf2->r &&
               trf->g == trf2->g && 
               trf->b == trf2->b && 
               trf->a == trf2->a &&
               tid == tid2)
                identical_line_counter_transparent ++;
            else
                identical_line_counter_transparent = 0;

            sb->identical_line_counter_transparent = identical_line_counter_transparent;
            sb->identical_line_counter = identical_line_counter;

            trf2 = trf;
            tid2 = tid;
        }
    }

    // y z
    face = 0;

    for(int i = 0; i < size[face][0]; i++)
    {
        identical_line_counter = 0;

        rf2 = &pos[face][i][0].render_flags;
        id2 = pos[face][i][0].block ? pos[face][i][0].block->id : 0;

        for(int j = 1; j < size[face][1]; j++)
        {
            sb = &pos[face][i][j];

            rf = &sb->render_flags;
            id = sb->block ? sb->block->id : 0;

            if(identical_line_counter < IDENDICAL_LINE_MAX && 
               rf->r == rf2->r &&
               rf->g == rf2->g && 
               rf->b == rf2->b && 
               rf->a == rf2->a &&
               id == id2)
                identical_line_counter ++;
            else
                identical_line_counter = 0;

            sb->identical_line_counter = identical_line_counter;

            rf2 = rf;
            id2 = id;

            rf2 = rf;
            id2 = id;

            trf = &sb->render_flags_transparent;
            tid = sb->transparent_block ? sb->transparent_block->id : 0;

            if(identical_line_counter_transparent < IDENDICAL_LINE_MAX && 
               trf->r == trf2->r &&
               trf->g == trf2->g && 
               trf->b == trf2->b && 
               trf->a == trf2->a &&
               tid == tid2)
                identical_line_counter_transparent ++;
            else
                identical_line_counter_transparent = 0;

            sb->identical_line_counter_transparent = identical_line_counter_transparent;
            sb->identical_line_counter = identical_line_counter;

            trf2 = trf;
            tid2 = tid;
        }
    }

    // std::cout << "finished !\n";
}