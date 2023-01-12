#version 430

// in vec4 color;

in vec2 texCoord;
uniform sampler2D world;

out vec4 fragColor;

void main (void)
{
    vec4 pixel = texture2D(world, texCoord);

    fragColor = pixel;

    gl_FragDepth = 0;
}