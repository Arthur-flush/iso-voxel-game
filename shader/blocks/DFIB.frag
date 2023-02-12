#version 430

//////// SHADER FEATURES ////////
#define SFEATURE_GLOBAL_ILLUMINATION 1
#define SFEATURE_AMBIANT_OCCLUSION   2
#define SFEATURE_BLOCK_BORDERS       4
#define SFEATURE_SHADOWS             8

#define SHADOW_TOP      128
#define SHADOW_LEFT      64
#define SHADOW_RIGHT     32

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 3) uniform vec3 light_direction;
layout (location = 4) uniform vec4 global_illumination;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;
layout (location = 7) uniform int block_size;
layout (location = 8) uniform int atlas_size;
layout (location = 9) uniform int max_height_render;

uint render_flagsl;
uint render_flagsr;
uint render_flagst;
uint shadows_flags;
vec2 texCoord;

uint id;
uint block_height;

in vec2 MozCoord;
in flat uint block_info;
in flat uint render_flags;

layout (location = 0) out vec4 fragColor;

layout (binding = 1) uniform sampler2D tex;
layout (binding = 2) uniform sampler2D ao;
layout (binding = 3) uniform sampler2D DFIB;
layout (binding = 4) uniform sampler2D water;
layout (binding = 5) uniform sampler2D border;
layout (binding = 6) uniform sampler2D normal;
layout (binding = 7) uniform sampler2D light;
layout (binding = 8) uniform sampler2D world;

float Ambiant_Oclusion(vec4 pixel_norm, vec4 pixel_AO)
{
    float PixelTint = 1;

    if(pixel_norm.g == 1)
    {
        if(
            ((render_flagsl&16) == 16 && texCoord.y < 0.60) || // top
            ((render_flagsl&8)  ==  8 && texCoord.y > 0.625)    // bottom
            
        ){
            PixelTint *= 1-pixel_AO.r*0.5;
        }

        if(
            ((render_flagsl&64) == 64 && texCoord.x < 0.25) || // left
            ((render_flagsr&64) == 64 && texCoord.x > 0.25)    // right
        ){
            PixelTint *= 1-pixel_AO.g*0.5;     
        }
        else
        if(
            ((render_flagst&2) == 2 && texCoord.x < 0.25 && texCoord.y < 0.60) || // corner top left
            ((render_flagsl&1) == 1 && texCoord.x < 0.25 && texCoord.y > 0.60) || // corner bottom left
            ((render_flagsr&1) == 1 && (render_flagsl&8) == 0 && texCoord.x > 0.25 && texCoord.y > 0.60)    // corner bottom right
        )
        {
            PixelTint *= 1-pixel_AO.r*pixel_AO.g*0.54;
        }
    }
    if(pixel_norm.r == 1)
    {
        if(
            ((render_flagsr&16) == 16 && texCoord.y < 0.60) || // top
            ((render_flagsr&8)  ==  8 && texCoord.y > 0.625)    // bottom
            
        ){
            PixelTint *= 1-pixel_AO.r*0.5;
        }
        
        if(
            ((render_flagsr&64) == 64 && texCoord.x < 0.75) || // left
            ((render_flagsr&32) == 32 && texCoord.x > 0.75)    // right
        ){
            PixelTint *= 1-pixel_AO.g*0.5; 
        }
        else
        if(
            ((render_flagst&1) == 1 && texCoord.x > 0.75 && texCoord.y < 0.60) || // corner top right
            ((render_flagsr&2) == 2 && texCoord.x > 0.75 && texCoord.y > 0.60) || // corner bottom right
            ((render_flagsr&1) == 1 && (render_flagsr&8) == 0 && texCoord.x < 0.75 && texCoord.y > 0.60)    // corner bottom left
        )
        {
            PixelTint *= 1-pixel_AO.r*pixel_AO.g*0.54;
        }
    }
    if(pixel_norm.b == 1)
    {   
        if(
            ((render_flagst&16) == 16 && texCoord.y > texCoord.x*0.5) || // bottom left
            ((render_flagst&32) == 32 && texCoord.y < texCoord.x*0.5)    // top right  
        ){
            PixelTint *= 1-pixel_AO.r*0.5;
        }
        
        if(
            ((render_flagst&8)  ==  8 && texCoord.y > (1-texCoord.x)*0.5) || // bottom right
            ((render_flagst&64) == 64 && texCoord.y < (1-texCoord.x)*0.5)    // top left
        ){
            PixelTint *= 1-pixel_AO.g*0.5;
        }
        
        if(PixelTint != 1.0)
            return PixelTint;
        else
        if(
            ((render_flagst&4) == 4 && texCoord.y < 0.15) || // corner top 
            ((render_flagst&2) == 2 && texCoord.x < 0.25) || // corner left
            ((render_flagst&1) == 1 && (render_flagst&32) == 0 && texCoord.x > 0.75)    // corner right
        )
        {
            PixelTint *= 1-pixel_AO.r*pixel_AO.g*0.54;
        }        
    }

    return PixelTint;
}


float Shadows(vec4 pixel_norm)
{
    // uint shadow_id = shadows_flags&(128+64+32);

    // if(shadow_id != 0)
    // {
    //     return 0.25;
    // }

    // if(id >= 240)
    // {
    //     if((shadows_flags&SHADOW_RIGHT) != 0 || (shadows_flags&SHADOW_LEFT) != 0 || (shadows_flags&SHADOW_TOP) != 0)
    //         return 0.5;
    // }

    if(
        pixel_norm.r == 1 && (shadows_flags&SHADOW_RIGHT) != 0 ||
        pixel_norm.g == 1 && (shadows_flags&SHADOW_LEFT)  != 0 ||
        pixel_norm.b == 1 && (shadows_flags&SHADOW_TOP)   != 0
    ){
        return 0.5;
    }
    return 1;
}


int handle_border(vec4 pixel_norm, float PixelTint)
{
    vec4 pixel_border = texture(border, texCoord);

    int border_color = 1;

    if(pixel_border.a > 0.99)
    {
        if(id >= 240)
        {
            return 0;
        }

        if(pixel_border.g == 1)
        {
            if(render_flagsl >= 128 && render_flagsr >= 128 && texCoord.x > 0.25 && texCoord.x < 0.75)
            {
                return border_color;
            }
        }

        if(pixel_border.r > 0.8)
        {
            if(render_flagsl >= 128 && render_flagst >= 128 && texCoord.y < 0.65)
            {
                return border_color;
            }
        }

        if(pixel_border.r < 1.0 && pixel_border.r > 0.0)
        {
            if(render_flagsr >= 128 && render_flagst >= 128 && texCoord.y < 0.65)
            {
                return border_color;
            }
        }


        if((render_flagsl&2) == 0)
        {
            if(pixel_border.b > 0.5)
            {
                return border_color;
            }

            if(pixel_border.g == 1 && texCoord.x < 0.25)
            {
                return border_color;
            }
        }

        if((render_flagsl&4) == 0)
        {
            if(pixel_border.b < 1 && pixel_border.b > 0)
            {
                return border_color;
            }

            if(pixel_border.g == 1 && texCoord.x > 0.75)
            {
                return border_color;
            }
        }

        if((render_flagsl&32) == 0)
        {
            if(pixel_border.r == 1 && texCoord.y > 0.65)
                return border_color;
        }

    }

    return 0;
}

void handle_depth()
{
    gl_FragDepth = 1-(float(block_height)+1.0)/16384; // 2^14
}

void main (void)
{
    vec4 pixel;

    texCoord.x = float(int(MozCoord.x*atlas_size)%block_size)/block_size;
    texCoord.y = float(int(MozCoord.y*atlas_size)%block_size)/block_size;

    pixel = texture(tex, MozCoord);

    vec4 pixel_norm = texture(normal, texCoord);

    if(pixel_norm.a == 0) discard; // discarding invisble fragments

    vec4 pixel_AO = texture(ao, texCoord);

    float PixelTint = 1.0;

    render_flagsl = render_flags%256;
    render_flagsr = (render_flags>>8)%256;
    render_flagst = (render_flags>>16)%256;
    shadows_flags = (render_flags>>24)%256;

    id = block_info%256;
    block_height = block_info>>8;

    handle_depth();

    //////////////////////// FACE CULLING /////////////////////////
    if(pixel_norm.g == 1 && render_flagsl < 128) discard;

    if(pixel_norm.r == 1 && render_flagsr < 128) discard;

    if(pixel_norm.b == 1 && render_flagst < 128)
    {
        PixelTint = 0.5;
    }
    ////////////////////////////////////////////////////////////////

    ///////////////////////// BASIC BORDER //////////////////////////
    int border = 0;
    if((features & SFEATURE_BLOCK_BORDERS) != 0)
    {
        border = handle_border(pixel_norm, PixelTint);
    }
    /////////////////////////////////////////////////////////////////

    /////////////////////////// SHADOWS ////////////////////////////
    if((features & SFEATURE_SHADOWS) != 0 && id < 224)
    {
        PixelTint *= Shadows(pixel_norm);
    }
    /////////////////////////////////////////////////////////////////

    /////////////////////// AMBIENT OCLUSION ///////////////////////
    if((features & SFEATURE_AMBIANT_OCCLUSION) != 0 && id < 224)
    {
        PixelTint *= Ambiant_Oclusion(pixel_norm, pixel_AO);
    }
    /////////////////////////////////////////////////////////////////

    fragColor.r = (block_height%256)/256.0;
    uint green = (block_height/256)*8;
    // green = 1;
    
    if(pixel_norm.g == 1)
        green += 1;

    else if(pixel_norm.b == 1)
        green += 2;

    green += border*4;

    // fragColor.g = (green & 127)/256.0;

    fragColor.g = green/256.0;

    fragColor.a = id/256.0;
    // fragColor.a = 1.0;

    fragColor.b = PixelTint;
}