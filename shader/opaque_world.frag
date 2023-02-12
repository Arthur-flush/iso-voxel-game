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

in vec2 texCoord;
layout (binding = 1) uniform sampler2D atlas;
layout (binding = 3) uniform sampler2D DFIB;
layout (binding = 7) uniform sampler2D light;

out vec4 fragColor;

uint id;
float PixelTint;
uint face;
int depth;
bool border;

void extract_id(vec4 DFIB)
{
    id = int(DFIB.a*256.0);
}

void extract_border(vec4 DFIB)
{
    uint b = int(DFIB.g*256.0) & 4;

    if(b == 0)
        border = false;
    else
        border = true;
}

float extract_Brightness(vec4 DFIB)
{
    return DFIB.b;
}

uint extract_face(vec4 DFIB)
{
    uint eface = (int(DFIB.g*256.0)<<30)>>30; // modulo didn't work here for some reasons

    if(eface != 0 && eface != 1)
        eface = 2;
    
    return eface;
}

int extract_depth(vec4 DFIB)
{
    return int(DFIB.r*256.0) + (int(DFIB.g*256.0)/8)*256;
}

bool test_border(vec2 uv)
{
    vec4 DFIB2 = texture(DFIB, uv);
    uint face2 = extract_face(DFIB2);

    if(face != face2)
    {
        return true;
    }

    int depth2 = extract_depth(DFIB2);

    if(abs(depth-depth2) > 3)
        return true;

    return false;
}

float handle_border2()
{
    float border_color = 1.5;
    float size = 0.000005* sprite_size;
    vec2 uv;

    uv = texCoord;
    uv.x += size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x -= size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.y += size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.y -= size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x += size;
    uv.y += size*2;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x -= size;
    uv.y += size*2;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x += size;
    uv.y -= size*2;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x -= size;
    uv.y -= size*2;
    if(test_border(uv)) return border_color;

    return 1.0;
}

float handle_border3()
{
    float border_color = 1.5;

    if(border)
        return border_color;

    return 1.0;
}

void main (void)
{
    /// debug
    // vec4 pl_debug = texture(light, texCoord);
    // fragColor = pl_debug;
    // return;

    ///// INIT DFIB /////
    vec4 DFIB = texture(DFIB, texCoord);
    extract_id(DFIB);
    if(id == 0) discard;

    ///// PIXEL TINT /////
    vec4 pixel_light = texture(light, texCoord);
    pixel_light *= 2;
    // pixel_light *= 0.5;
    // pixel_light = sqrt(pixel_light);

    // float maxc = max(pixel_light.r, pixel_light.g);
    // maxc = max(maxc, pixel_light.b);

    // float lim = 0.95;
    // if(pixel_light.b > lim) pixel_light.b = lim;
    // if(pixel_light.r > lim) pixel_light.g = lim;
    // if(pixel_light.g > lim) pixel_light.b = lim;

    face = extract_face(DFIB);

    // float GI = 1.0;
    float GI = light_direction[face];

    if(id >= 224)
        GI = GI + (1.0-GI)*0.85;

    // vec3 test = global_illumination.rgb + pixel_light.rgb*0.5;
    vec3 test = global_illumination.rgb + pixel_light.rgb*0.5;

    if(test.r >= 1) test.r = 1;
    if(test.g >= 1) test.g = 1;
    if(test.b >= 1) test.b = 1;

    vec4 PixelTint = vec4(test, 1);
    float b = extract_Brightness(DFIB);

    // if(b <= 1.0)
    //     b += (pixel_light.r + pixel_light.g + pixel_light.b)/3.0 * pixel_light.a * 5;
    // if(b > 1.0) b == 1.0;

    // float val = (pixel_light.r + pixel_light.g + pixel_light.b)/3.0;
    // if(val != 0.0)
    //     b *= 1.0+val;

    
    PixelTint *= b;
    PixelTint *= GI;

    ///// BORDER /////
    depth = extract_depth(DFIB);
    extract_border(DFIB);
    PixelTint *= handle_border3();

    ///// COLOR /////
    vec2 ColorCoord = vec2(float(id%16)/16.0, float(id/16)/16.0);
    vec4 block_color = texture(atlas, ColorCoord);

    ///// FINAL /////

    // pixel_light = pixel_light - PixelTint;

    // PixelTint *= 1.0 + pixel_light;

    fragColor = block_color * PixelTint;

    fragColor.a = 1.0;
    gl_FragDepth = 0;

    //debug
    // fragColor.rgb = vec3(depth/512.0);
}