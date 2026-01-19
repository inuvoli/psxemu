#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "GL/glew.h"
#include "glm/glm.hpp"

class Shader
{
    public:
        Shader(const GLchar* vertexSourcePath, const GLchar* fragmentSourcePath);

        void Use();
        void setUniformb(const std::string &name, bool value) const;
        
        void setUniformi(const std::string &name, int value) const;
        void setUniformi(const std::string &name, const glm::ivec2& value) const;
        void setUniformi(const std::string &name, const glm::ivec3& value) const;
        void setUniformi(const std::string &name, const glm::ivec4& value) const;

        void setUniformui(const std::string &name, unsigned int value) const;
        void setUniformui(const std::string &name, const glm::uvec2& value) const;
        void setUniformui(const std::string &name, const glm::uvec3& value) const;
        void setUniformui(const std::string &name, const glm::uvec4& value) const;
        
        void setUniform(const std::string &name, float value) const;
        void setUniform(const std::string &name, const glm::vec2& value) const;
        void setUniform(const std::string &name, const glm::vec3& value) const;
        void setUniform(const std::string &name, const glm::vec4& value) const;

    private:
        GLuint programId;      
};

