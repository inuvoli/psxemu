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

        //GLfloat colors[256] = { 255.0f,  0.0f,  0.0f, 0.0f,  255.0f,  0.0f, 0.0f,  0.0f,  255.0f };
        //GLfloat vertices[256] = { 0.0f, 480.0f, 640.0f, 480.0f, 320.0f, 0.0f } ;
        GLuint colors[256] = { 255,  0,  0, 0,  255,  0, 0,  0,  255 };
        GLint vertices[256] = { 0, 480, 640, 480, 320, 0 } ;
        GLuint VBO, CBO, VAO;

};  