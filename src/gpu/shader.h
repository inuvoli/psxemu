#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "GL/glew.h"

class Shader
{
    public:
        Shader(const GLchar* vertexSourcePath, const GLchar* fragmentSourcePath);

        void Use();
        void setUniformb(const std::string &name, bool value) const;
        void setUniformi(const std::string &name, int value) const;
        void setUniformui(const std::string &name, unsigned int value) const;
        void setUniformf(const std::string &name, float value) const;

    public:
        GLuint programId;      
};

