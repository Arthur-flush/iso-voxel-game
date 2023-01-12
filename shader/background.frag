#version 430

layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;

in vec2 texCoord;
layout (location = 6) uniform sampler2D iChannel0;

out vec4 fragColor;

void main (void)
{
    vec4 pixel = texture2D(iChannel0, texCoord);
    fragColor = pixel;

    gl_FragDepth = 0;
}