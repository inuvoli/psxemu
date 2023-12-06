#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "shader.h"

#include "GL/glew.h"
#include "glm/glm.hpp"

constexpr auto maxVertices = 1024 * 512;

struct VertexInfo
{
    glm::vec3       vertexColor;
    glm::vec2       vertexPosition;
    glm::vec2       vertexTexCoords;
    glm::vec2       clutTableCoords;
    glm::float32    textured;
    glm::vec2       texPageCoords;
    glm::float32    texColorDepth;
    glm::float32    texBlending;
    glm::float32    transparent;
    glm::float32    transMode;
    
};

class Renderer
{
    public:
        //Renderer();
        Renderer(const uint16_t* pData); 
        ~Renderer();

        bool reset();
     
        bool InsertPolygon(std::vector<VertexInfo>& e);
        bool InsertLine(std::vector<VertexInfo>& e);
        
        bool SetResolution(int16_t hRes, int16_t vRes);

        bool NewFrameReady();               //Called from GPU at every vBlank, update OpenGL Vertex Arrays and vRAM Texture Data 
        bool RenderDrawData();              //Called from PSX at every NewFrame, render all Vertex Array to Default Frame Buffer 

    public:


    private:
        std::vector<VertexInfo> drawData;   //Contains all Vertex Infos, copied on VBO at every vBlank
        int drawDataVertexNumber;           //Contains the number of Vertex Stored on VBO at every vBlank

        uint16_t* textureData;              //Pointer to PBO Memory Area
        const uint16_t* videoRam;           //Pointer to GPU Video Ram
        std::shared_ptr<Shader> pShader;
        GLuint VAO;
        GLuint VBO;
        GLuint PBO;
        GLuint textureId;

        //Helper Arrays
        static constexpr auto DEFAULT_HRES = 256.0f;
        static constexpr auto DEFAULT_VRES = 240.0f; 
};  