#version 460 core

layout(location = 0) in vec3 vertexColor;
layout(location = 1) in vec2 vertexPosition;
layout(location = 2) in vec2 vertexTexCoords;
layout(location = 3) in vec2 clutTableCoords;
layout(location = 4) in float textured;
layout(location = 5) in vec2 texPageCoords;
layout(location = 6) in float texColorDepth;
layout(location = 7) in float texBlending;
layout(location = 8) in float transparent;
layout(location = 9) in float transMode;

out vec3 vColor;
out vec2 vTextureCoords;
out vec2 clutTable;
out float textureOn;
out vec2 texturePage;
out float textureColorDepth;
out float textureBlending;
out float transOn;
out float transAlgo;

uniform float hRes;
uniform float vRes;

void main()
{
    //Normalize Colors
    vColor = vertexColor / 255.0f;

    //Incapsulate Vertex Texture and Page Texture Coordinates
    vTextureCoords = vertexTexCoords;
    clutTable = clutTableCoords;
    textureOn = textured;
    texturePage = texPageCoords;
    textureColorDepth = texColorDepth;
    textureBlending = texBlending;
    transOn = transparent;
    transAlgo = transMode;
 
    //Tranform PSX coordinates to OpenGL Coordinates (-1, +1)
    float xpos = (vertexPosition.x / hRes * 2) - 1.0f;
    float ypos = 1.0f - (vertexPosition.y / vRes *2);     
    gl_Position = vec4(xpos, ypos, 0.0f, 1.0f);
}
