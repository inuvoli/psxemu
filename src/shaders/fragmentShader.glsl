#version 460 core
in vec3 vertexColor;

out vec4 fragColor;

//uniform usampler2DRect vram;

void main()
{
    fragColor.xyz = vertexColor;
    fragColor.w = 1.0f;
}