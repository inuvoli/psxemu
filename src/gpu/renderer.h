#pragma once

#include <cstring>
#include <cstdint>
#include <memory>
#include <vector>
#include <span>

#include "GL/glew.h"
#include "glm/glm.hpp"

#include "shader.h"
#include "gpu_utils.h"

static constexpr size_t MAX_VERTICES = 65536;

struct RendererVertex
{
    glm::vec2 pos;      // Vertex coordinate (PSX Format)
    glm::vec3 color;    // Vertex RGB Color (PSX Format)
    glm::vec2 uv;       // Vertex Texture Coordinates (PSX Format)
};

struct VideoState
{
    glm::ivec2  displayRes;
    glm::ivec4  displayArea;
    int         displayColorMode;
    bool        displayDisabled;
    bool        interlaced;
    bool        evenfield;

    bool operator==(const VideoState& o) const
    {
        return memcmp(this, &o, sizeof(VideoState)) == 0;
    }

};

struct RendererState
{
    glm::ivec4  drawingArea;
    glm::ivec2  drawingOffset;
    bool        drawingEnabled;
    glm::ivec2  texPageCoords;
    glm::ivec2  texMask;
    glm::ivec2  texOffset;
    glm::ivec2  clutTableCoords;
    int         texColorMode;
    bool        textured;
    bool        texBlending;
    bool        semiTranparent;
    int         semiTransparentMode;
    bool        checkMask;
    bool        forceMask; 
    bool        dither;
   
    bool operator==(const RendererState& o) const
    {
        return memcmp(this, &o, sizeof(RendererState)) == 0;
    }
};

class Renderer
{
    public:
        static Renderer& instance()
        {
            static Renderer *instance = new Renderer();
            return *instance;
        }
    
        //Renderer Interface
        static bool Init();
        static bool Reset();
        static bool FrameReady();
        static void vBlank();
        static void ScreenUpdate();
        
        //Set Video Display Status
        static void SetDisplayArea(lite::vec4t<uint16_t> displayArea);
        static void SetDisplayDisable(bool disabled);
        static void SetDisplayResolution(lite::vec2t<uint16_t> resolution);
        static void SetDisplayColorMode(uint8_t colorMode);
        static void SetDisplayInterlaceMode(bool interlaced);
        
        //Set Renderer Status
        static void SetDrawingArea(lite::vec4t<uint16_t> drawingArea);
        static void SetDrawingOffset(lite::vec2t<uint16_t> drawingOffset);
        static void SetDrawingEnabled(bool drawingEnable);
        static void SetTexturePage(lite::vec2t<uint16_t> texPageCoords);
        static void SetClutTable(lite::vec2t<uint16_t> clutCoords);
        static void SetTextureColorMode(uint8_t colorMode);
        static void SetTextureMode(bool textured, bool texBlending);
        static void SetTextureMask(lite::vec2t<uint8_t> textureMask);
        static void SetTextureOffset(lite::vec2t<uint8_t> textureOffset);
        static void SetTransparency(bool semiTransparent);
        static void SetTransparencyMode(int semiTransparentMode);
        static void SetMaskBit(bool checkMask, bool forceMask);
        static void SetDither(bool dither);
        
        //Draw Polygon Functions
        static void DrawPolygon(GpuVertex *vertex, uint16_t vertexNum);
        static void DrawRectangle(GpuVertex *vertex);
        static void DrawPoint(GpuVertex *vertex);
        static void DrawLine(GpuVertex *vertex);

        //VRAM Access Functions
        static bool CommitVRAMWrite();
        static bool WriteVRAM(uint16_t x, uint16_t y, uint16_t data);
        static uint16_t ReadVRAM(uint16_t x, uint16_t y);
        static GLuint GetVRAMTextureObject();


    private:
        Renderer() {}

        void BeginBatch();
        void FlushBatch();
        void PushVertex(const RendererVertex& v);
        void SetupRenderShader();
        void SetupFramebufferShader();
        void UpdateDebugTexture();
                
        //Frame Status
        bool                    frameReady;

        //Renderer State
        VideoState              videoState;                 //Current Video Display configuration
        RendererState           currentState;               //Current State being set
        RendererState           renderingState;             //Actual State used for rendering


        //Internal Structures
        GLuint                  vaoRenderBuffer;            //VAO for Rendering
        GLuint                  vboRenderBuffer;            //Vertex Persistent Buffer Object for Rendering
        GLuint                  vaoFrameBuffer;             //VAO for Framebuffer Quad
        GLuint                  vboFrameBuffer;             //Vertex Buffer Object for Framebuffer Quad
        GLuint                  vramTexture;                //Texture Object representing PSX VRAM
        GLuint                  vramBuffer;                 //VRAM Persistent Buffer Object
        GLuint                  vramFrameBuffer;            //Frame Buffer Object for Primitive Rendering
        GLuint                  vramDebugTexture;

        //Persistent Vertex Buffer for Rendering Primitives
        RendererVertex*         mappedVertex = nullptr;     //Mapped Vertex Container, cointains the actual data.
        int                     vertexCount = 0;            //Number of Vertex on the current Batch
        GLsync                  fence = nullptr;            //OpenGL Sync object

        uint16_t*               mappedVRAM;                 //Mapped Object Container, contains the actual VRAM data

        std::unique_ptr<Shader> FramebufferShader;          //Framebuffer Rendering Shader Program   
        std::unique_ptr<Shader> RenderShader;               //Primitive Rendering Shader Program

        //Helper Arrays
        static constexpr auto DEFAULT_HRES = 256.0f;
        static constexpr auto DEFAULT_VRES = 240.0f; 
};  

