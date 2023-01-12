#version 430

layout (triangles) in;
layout (triangle_strip, max_vertices = 128) out;

uniform mat4 gpu_ModelViewProjectionMatrix;
uniform sampler2D tex;

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 3) uniform vec3 light_direction;
layout (location = 4) uniform vec4 global_illumination;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;
layout (location = 7) uniform int block_size;
layout (location = 8) uniform int atlas_size;

in vec2 VMozCoord[];
in vec4 Vcolor[];
in ivec2 corner[];
in vec3 final_Vertex[];

in flat int Vid[];
in ivec2 block_position[];
in flat uint Vblock_height[];
in flat int Vline_size[];

int render_flagsl;
int render_flagsr;
int render_flagst;
int shadows_flags;
int id;

int projection_grid_face;
int line_size;

out vec2 MozCoord;
out flat uint block_info;
out flat uint render_flags;

void manage_outpouts(int i)
{
    id = Vid[2];
    render_flagsl = int(Vcolor[i].r*256);
    render_flagsr = int(Vcolor[i].g*256);
    render_flagst = int(Vcolor[i].b*256);
    shadows_flags = int(Vcolor[i].a*256);

    block_info = id + (Vblock_height[0]<<8);

    render_flags = render_flagsl + (render_flagsr<<8) + (render_flagst<<16) + (shadows_flags<<24) ;
}

void get_line_size()
{
    line_size = 1;
    for(int i = 0; i < 3; i++)
        if(Vline_size[i] != 0)
            line_size = Vline_size[i] + 1;
}

void determine_projection_grid_face()
{
    projection_grid_face = shadows_flags%4;
}

void main()
{
    manage_outpouts(0);
    get_line_size();
    // line_size = 1;
    determine_projection_grid_face();

    for(int i = 0; i < line_size; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            vec3 VPos = final_Vertex[j];

            if(projection_grid_face == 2)
            {
                // axe y
                VPos.x += (sprite_size/2)*(-i);
                VPos.y += (sprite_size/4)*(i);
            }
            if(projection_grid_face == 1)
            {
                // axe x
                VPos.x += (sprite_size/2)*(i);
                VPos.y += (sprite_size/4)*(i);
            }
            if(projection_grid_face == 0)
            {
                // axe z
                VPos.y += (sprite_size/2)*(i);
            }

            gl_Position = gpu_ModelViewProjectionMatrix * vec4(VPos, 1);

            MozCoord = VMozCoord[j];
            EmitVertex();
        }

        EndPrimitive();
    }
}