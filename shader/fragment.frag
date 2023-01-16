#version 430

// layout(early_fragment_tests) in;

//////// SHADER FEATURES ////////
#define SFEATURE_GLOBAL_ILLUMINATION 1
#define SFEATURE_AMBIANT_OCCLUSION   2
#define SFEATURE_BLOCK_BORDERS       4
#define SFEATURE_SHADOWS             8

#define SHADOW_TOP      128
#define SHADOW_LEFT     64
#define SHADOW_RIGHT    32
#define HIDE_PART_LEFT    4
#define HIDE_PART_RIGHT   2
#define HIDE_PART_BOTTOM  1

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 3) uniform vec3 light_direction;
layout (location = 4) uniform vec4 global_illumination;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;
layout (location = 7) uniform int block_size;
layout (location = 8) uniform int atlas_size;

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

out vec4 fragColor;

uniform sampler2D tex;
uniform sampler2D normal;
uniform sampler2D ao;
uniform sampler2D world;
uniform sampler2D water;

float Ambiant_Oclusion(vec4 pixel_norm, vec4 pixel_AO)
{
    float PixelTint = 1;

    if(pixel_norm.g == 1)
    {
        if(
            ((render_flagsl&16) == 16 && texCoord.y < 0.60) || // top
            ((render_flagsl&8)  ==  8 && texCoord.y > 0.60)    // bottom
            
        ){
            PixelTint *= 1-pixel_AO.r*0.5;
        }

        if(
            ((render_flagsl&64) == 64 && texCoord.x < 0.25) || // left
            ((render_flagsl&32) == 32 && texCoord.x > 0.25)    // right
        ){
            PixelTint *= 1-pixel_AO.g*0.5;     
        }
        else
        if(
            ((render_flagsl&4) == 4 && texCoord.x < 0.25 && texCoord.y < 0.60) || // corner top left
            ((render_flagsl&1) == 1 && texCoord.x < 0.25 && texCoord.y > 0.60) || // corner bottom left
            ((render_flagsl&2) == 2 && texCoord.x > 0.25 && texCoord.y > 0.60)    // corner bottom right
        )
        {
            PixelTint *= 1-pixel_AO.r*pixel_AO.g*0.54;
        }
    }
    if(pixel_norm.r == 1)
    {
        if(
            ((render_flagsr&16) == 16 && texCoord.y < 0.60) || // top
            ((render_flagsr&8)  ==  8 && texCoord.y > 0.60)    // bottom
            
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
            ((render_flagsr&4) == 4 && texCoord.x > 0.75 && texCoord.y < 0.60) || // corner top right
            ((render_flagsr&2) == 2 && texCoord.x > 0.75 && texCoord.y > 0.60) || // corner bottom right
            ((render_flagsr&1) == 1 && texCoord.x < 0.75 && texCoord.y > 0.60)    // corner bottom left
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
        else
        if(
            ((render_flagst&4) == 4 && texCoord.y < 0.15) || // corner top 
            ((render_flagst&2) == 2 && texCoord.x < 0.25) || // corner left
            ((render_flagst&1) == 1 && texCoord.x > 0.75)    // corner right
        )
        {
            PixelTint *= 1-pixel_AO.r*pixel_AO.g*0.54;
        }        
    }

    return PixelTint;
}

float Shadows(vec4 pixel_norm)
{
    if(
        pixel_norm.r == 1 && (shadows_flags&SHADOW_RIGHT) != 0 ||
        pixel_norm.g == 1 && (shadows_flags&SHADOW_LEFT)  != 0 ||
        pixel_norm.b == 1 && (shadows_flags&SHADOW_TOP)   != 0
    ){
        return 0.5;
    }
    return 1;
}

// OLD BUT CAN BE USEFULL ONE DAY
float near = 0.1f;
float far = 100.0f;
float linearizeDepth(float depth)
{
    float ret = (2.0*near*far)/(far+near-(depth*2.0-1.0)*(far-near));
    return ret;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 u_WaveStrengthX=vec4(4.15,4.66,0.0016,0.0015)*log(sprite_size)/7.0;
vec4 u_WaveStrengthY=vec4(2.54,6.33,0.00102,0.0025)*log(sprite_size)/7.0;
const float iTime = 0.5;
vec2 dist(vec2 uv)
{ 
    float uTime = Time;
    if(uTime==0.0) uTime=0.15*iTime;
    float noise = texture(water, uTime + uv).r;
    uv.y += (cos((uv.y + uTime * u_WaveStrengthY.y + u_WaveStrengthY.x * noise)) * u_WaveStrengthY.z) +
        (cos((uv.y + uTime) * 10.0) * u_WaveStrengthY.w);

    uv.x += (sin((uv.y + uTime * u_WaveStrengthX.y + u_WaveStrengthX.x * noise)) * u_WaveStrengthX.z) +
        (sin((uv.y + uTime) * 15.0) * u_WaveStrengthX.w);
    return uv;
}

void handle_water(vec4 pixel)
{
    float val = (render_flagsl%32)/16.0;

    // if(val > 1)
        pixel.g = pixel.g + ((2-val)/16.0);

    /// water color desaturation
    float desaturation_value = ((1-val)/6.0); // version colorée
    //float desaturation_value = ((2-val)/4.0); // version réaliste
    vec3 pixel_hsv = rgb2hsv(pixel.rgb);
    pixel_hsv.y = pixel_hsv.y-desaturation_value;
    vec3 new_pixel = hsv2rgb(pixel_hsv.rgb);
    pixel.rgb = new_pixel.rgb;
    //////////////////////////////////////

    pixel.a += (val/5.0); // := water opacity
    if(pixel.a > 1)
        pixel.a = 1;  

    vec2 screen_pos = vec2(gl_FragCoord.x/1920, gl_FragCoord.y/1080);
    screen_pos = dist(screen_pos);
    vec4 pixel_world = texture(world, screen_pos);


    pixel.rgb = rgb2hsv(pixel.rgb);
    pixel_world.rgb = rgb2hsv(pixel_world.rgb);

    // pixel_world.r = (pixel.r + pixel_world.r)/2;

    float color_shift_value = pixel_world.g*(2-val)*(2-val)/16.0;
    pixel.r = (pixel.r*pixel.a + pixel_world.r*color_shift_value)/(pixel.a+color_shift_value);
    // pixel.r = (pixel.r + pixel_world.r*color_shift_value)/(1+color_shift_value);

    // pixel_world.r = pixel.r*color_shift_value + pixel_world.r*(4-color_shift_value);

    pixel.rgb = hsv2rgb(pixel.rgb);
    pixel_world.rgb = hsv2rgb(pixel_world.rgb);

    fragColor.rgb = pixel.rgb*pixel.a + (pixel_world.rgb * (1-pixel.a));

    fragColor.a = 1;
}

// https://thebookofshaders.com/11/
// 2D Random
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

const float Persistance = 1;
const float Roughness = 0.2;

float diffraction(vec2 world_pos) {

    float diffraction_index = 0;
    float frequency = 1;
    float factor = 1;

    for (int i = 0; i < 4; i++) {
        diffraction_index += noise(world_pos * frequency / (sprite_size / 2048) * i * 0.72354) * factor;
        factor *= Persistance;
        frequency *= Roughness;
    }

    // diffraction_index = ((noise((world_pos * 0.1) / (sprite_size / 2048)) + noise((world_pos * 1) / (sprite_size / 2048)) *0.2 )) * 0.01; 
    return diffraction_index;
}

float test(vec2 v) {
    return sqrt(v.x*v.x + v.y*v.y) / 2;
}

void handle_glass(vec4 pixel) {
    vec2 screen_pos = vec2(gl_FragCoord.x/1920, gl_FragCoord.y/1080);
    vec2 world_pos = vec2(
        screen_pos.x - (win_const.z/1920.0), 
        screen_pos.y - (win_const.w/1080.0)
        );
    vec2 diffracted_pos = screen_pos + vec2(diffraction(world_pos)) * (sprite_size / 2048);
    vec4 pixel_world = texture(world, diffracted_pos);


    // fragColor.rgb = vec3(diffraction(world_pos)) * 50;
    
    fragColor.rgb = pixel_world.rgb;

    // fragColor.rgb = pixel.rgb*pixel.a + (pixel_world.rgb * (1-pixel.a));

    // fragColor.rgb = vec3(test(diffracted_pos));

    fragColor.a = 1;
}

void handle_depth()
{
    gl_FragDepth = 1-(block_height*1.0+1.0)/atlas_size;
}

void main (void)
{
    vec4 pixel;
    gl_FragDepth = 1; // 1

    texCoord.x = float(int(MozCoord.x*atlas_size)%block_size)/block_size;
    texCoord.y = float(int(MozCoord.y*atlas_size)%block_size)/block_size;

    pixel = texture(tex, MozCoord);
    // pixel = vec4(0.5, 0.5, 0.5, 1.0); // debug

    if(pixel.a == 0) discard; // discarding invisble fragments

    render_flagsl = render_flags%256;
    render_flagsr = (render_flags>>8)%256;
    render_flagst = (render_flags>>16)%256;
    shadows_flags = (render_flags>>24)%256;

    id = block_info%256;
    block_height = block_info>>8;

    vec4 PixelTint = vec4(global_illumination.xyz, 1);

    vec4 pixel_norm = texture(normal, texCoord);

    vec4 pixel_AO = texture(ao, texCoord);

    //////////////////////// FACE CULLING /////////////////////////
    if(pixel_norm.r == 1 && render_flagsr < 128) discard;

    if(pixel_norm.g == 1 && render_flagsl < 128) discard;
    
    if(pixel_norm.b == 1 && render_flagst < 128) discard;
    ////////////////////////////////////////////////////////////////

    /////////////////////////// SHADOWS ////////////////////////////
    if((features & SFEATURE_SHADOWS) != 0)
    {
        PixelTint *= Shadows(pixel_norm);
    }
    /////////////////////////////////////////////////////////////////

    //////////////////// TRANSPARENT FRAGMENTS /////////////////////
    if(pixel.a > 0 && pixel.a != 1)
    {
        handle_depth();
        fragColor = pixel;

        if(id == 240) // water
        {
            handle_water(pixel);
        }
        else if (id == 241) { // glass
            handle_glass(pixel);
        }


        // PixelTint.rgb = vec3(0.5);
        fragColor.rgb *= vec3(PixelTint);

        return;
    }
    ////////////////////////////////////////////////////////////////

    /////////////////////// AMBIENT OCLUSION ///////////////////////
    if((features & SFEATURE_AMBIANT_OCCLUSION) != 0)
    {
        PixelTint *= Ambiant_Oclusion(pixel_norm, pixel_AO);
    }
    /////////////////////////////////////////////////////////////////

    ///////////////////////// BASIC BORDER //////////////////////////

    /////////////////////////////////////////////////////////////////
    
    ///////////////////// GLOBAL ILLUMINATION  //////////////////////
    if((features & SFEATURE_GLOBAL_ILLUMINATION) == SFEATURE_GLOBAL_ILLUMINATION)
    {
        PixelTint *= dot(light_direction, vec3(pixel_norm.r, pixel_norm.g, pixel_norm.b));
    }
    /////////////////////////////////////////////////////////////////

    // pixel = vec4(render_flagsl/256.0, render_flagsr/256.0, render_flagst/256.0, 1.0);

    PixelTint.a = 1;
    handle_depth();
    fragColor = pixel * PixelTint;
}