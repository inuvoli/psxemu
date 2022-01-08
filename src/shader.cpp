#include "shader.h"

Shader::Shader(const GLchar* vertexSourcePath, const GLchar* fragmentSourcePath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    //check if files can throw an exception
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try
    {
        //Open files
        vShaderFile.open(vertexSourcePath);
        fShaderFile.open(fragmentSourcePath);
        std::stringstream vShaderStream, fShaderStream;

        //Read file
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        //Close file
        vShaderFile.close();
        fShaderFile.close();

        //Convert Stream to GLchar
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch(const std::exception& e)
    {
        std:cout << "ERROR::SHADER::FILE_NOT_READ" << std::endl;
    }

    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar* fShaderCode = fragmentCode.c_str();

    GLuint vertex, fragment;
    GLint success;
    GLchar infoLog[512];

    //Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILES\n" << infoLog << std::endl;
    }

    //Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILES\n" << infoLog << std::endl;
    }

    //Shader Program
    this->programId = glCreateProgram();
    glAttachShader(this->programId, vertex);
    glAttachShader(this->programId, fragment);
    glLinkProgram(this->programId);
    glGetProgramiv(this->programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(this->programId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    //Delete shader
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::Use()
{
    glUseProgram(this->programId);
}

void Shader::setUniformb(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(this->programId, name.c_str()), (int)value); 
}

void Shader::setUniformi(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(this->programId, name.c_str()), value); 
}

void Shader::setUniformf(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(this->programId, name.c_str()), value); 
} 
