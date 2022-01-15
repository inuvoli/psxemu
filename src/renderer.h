#pragma once

#include "shader.h"
#include "common.h"

#include "GL/glew.h"

constexpr auto DEFAULT_HRES = 256;
constexpr auto DEFAULT_VRES = 240;


class Renderer
{
    public:
        Renderer();
        ~Renderer();
     
        bool renderPolygon(uint8_t numVertex, const void* vertices, const void* colors);
        bool renderLine();
        bool renderRectangle();
        bool rendererSetResolution(int16_t hRes, int16_t vRes);

    public:
        Shader* pShader;
        GLuint VBO, CBO, VAO, EBO;     
};  