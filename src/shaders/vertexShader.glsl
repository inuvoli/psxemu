#version 460 core

layout (location = 0) in ivec2 position;
layout (location = 1) in uvec3 color;

out vec3 vertexColor;

uniform int hRes = 1024;
uniform int vRes = 512;

void main()
{
    //Tranform Integer PSX coordinates to OpenGL Coordinates (-1, +1)
    float xpos = (float(position.x) / float(hRes) * 2) - 1.0f;
    float ypos = 1.0f - (float(position.y) / float(vRes) * 2);
    
    gl_Position = vec4(xpos, ypos, 0.0f, 1.0f);

    //Normalize Colors
    vertexColor = vec3(color) / 255.0f;
}