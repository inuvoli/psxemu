#pragma once

#include "shader.h"

#include "GL/glew.h"

class Renderer
{
    public:
        Renderer();
        ~Renderer();

        bool renderFrame();
        bool updateDrawData();

    public:
        Shader* pShader;

        GLfloat colors[256] = { 1.000f,  0.000f,  0.000f, 0.000f,  1.000f,  0.000f, 0.000f,  0.000f,  1.000f };
        GLfloat vertices[256] = { 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f } ;
        GLuint VBO, CBO, VAO;

};