#version 430

in vec3 gpu_Vertex;
in vec2 gpu_TexCoord;
in vec4 gpu_Color;
uniform mat4 gpu_ModelViewProjectionMatrix;

// out vec4 color;
out vec2 texCoord;

void main (void)
{
    texCoord = gpu_TexCoord;
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 1);
}