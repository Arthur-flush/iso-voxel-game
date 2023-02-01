#version 430

#define SFEATURE_BLOOM 16
#define SFEATURE_GRID  32

layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;

in vec2 texCoord;
uniform sampler2D iChannel0;

out vec4 fragColor;

// bloom actuel : https://www.shadertoy.com/view/lsXGWn
// pk pas tester : https://www.shadertoy.com/view/MtfSDH ou https://www.shadertoy.com/view/flK3zy ou https://www.shadertoy.com/view/lsXGWn
const float blurSize = 1.0/512.0;
const float intensity = 0.10;
void bloom()
{
   vec4 sum = vec4(0);
   int j;
   int i;

   //thank you! http://www.gamerendering.com/2008/10/11/gaussian-blur-filter-shader/ for the 
   //blur tutorial
   // blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture(iChannel0, vec2(texCoord.x - 4.0*blurSize, texCoord.y)) * 0.05;
   sum += texture(iChannel0, vec2(texCoord.x - 3.0*blurSize, texCoord.y)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x - 2.0*blurSize, texCoord.y)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x - blurSize, texCoord.y)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y)) * 0.16;
   sum += texture(iChannel0, vec2(texCoord.x + blurSize, texCoord.y)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x + 2.0*blurSize, texCoord.y)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x + 3.0*blurSize, texCoord.y)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x + 4.0*blurSize, texCoord.y)) * 0.05;
	
	// blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - 4.0*blurSize)) * 0.05;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - 3.0*blurSize)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - 2.0*blurSize)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - blurSize)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y)) * 0.16;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + blurSize)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + 2.0*blurSize)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + 3.0*blurSize)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + 4.0*blurSize)) * 0.05;

   //increase blur with intensity!
   fragColor = sum*intensity + texture(iChannel0, texCoord); 
   
   /*
   if(sin(iTime) > 0.0)
       fragColor = sum * sin(iTime)+ texture(iChannel0, texCoord);
   else
	   fragColor = sum * -sin(iTime)+ texture(iChannel0, texCoord);
    */
}

// void is_near(vec2 value, vec2 target, float nprecision)
// {
//     if(value.x > target.x-nprecision && value.x < target.x-nprecision)
// }

void isometric_grid()
{
    vec2 pixel_size = vec2(1.0/1920, 1.0/1080);

    if((texCoord.x > 0.5-pixel_size.x && texCoord.x < 0.5+pixel_size.x) ||
       (texCoord.y > 0.5-pixel_size.y && texCoord.y < 0.5+pixel_size.y))
    {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }


    // float face_with = sprite_size/2.0;
    // float face_height = sprite_size/4.0;

    // // y = 0.5x
    // // y/x = 0.5
    // // x/y = 2

    // vec2 norm_coord = vec2(texCoord.x*float(win_const.x), texCoord.y*float(win_const.y));

    // float test = norm_coord.y/norm_coord.x;

    // // if(test > 1 || test < 3)
    // if(test != 0.0)
    // {
    //     fragColor = vec4(1.0, 1.0, 0.0, 1.0);
    //     return;
    // }

    // fragColor = vec4(test, test, test, 1.0);
}

void main (void)
{

    vec4 pixel = texture(iChannel0, texCoord);
    fragColor = pixel;

    if((features & SFEATURE_BLOOM) != 0)
        bloom();

    if((features & SFEATURE_GRID) != 0)
        isometric_grid();

    // fragColor.rgb -= mod(gl_FragCoord.y, 2.0) < 1.0 ? 0.25 : 0.0;

    gl_FragDepth = 0;
}