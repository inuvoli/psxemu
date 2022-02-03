#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "shader.h"
#include "vectors.h"

#include "GL/glew.h"

struct VertexInfo
{
    vec2t<float> pos;
};

class Renderer
{
    public:
        //Renderer();
        Renderer(const uint16_t* pData); 
        ~Renderer();
     
        bool DrawPolygon(uint8_t numVertex, const void* vertices, const void* colors, const void* texture, bool textured, uint16_t clutInfo, uint16_t texPageInfo);
        bool DrawLine();
        bool DrawRectangle();
        bool SetResolution(int16_t hRes, int16_t vRes);
        bool UpdateFrame();

    public:
        uint16_t* textureData;
        const uint16_t* videoRam;
        std::shared_ptr<Shader> pShader;

        GLuint VBO, CBO, VAO, EBO, TBO, PBO, textureId;

        //Helper Arrays
        static constexpr GLuint polygonIndices[6] = {0, 1, 2, 1, 2, 3};
        static constexpr auto DEFAULT_HRES = 256;
        static constexpr auto DEFAULT_VRES = 240;

    private:
        std::vector<VertexInfo> drawData; 
};  