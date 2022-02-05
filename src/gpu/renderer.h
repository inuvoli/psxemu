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
    glm::vec2       texPageCoords;
    glm::float32    texColorDepth;
    glm::float32    textured;
    // glm::float32    transparent;
    // glm::float32    blending;
};

class Renderer
{
    public:
        //Renderer();
        Renderer(const uint16_t* pData); 
        ~Renderer();
     
        bool DrawPolygon(std::vector<VertexInfo>& e);
        bool DrawLine();
        bool DrawRectangle();
        bool SetResolution(int16_t hRes, int16_t vRes);
        bool RenderPolygons();
        bool RenderFrame();

    public:
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

    private:
        std::vector<VertexInfo> drawData; 
};  