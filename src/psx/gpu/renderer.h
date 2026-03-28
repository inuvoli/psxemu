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

static constexpr auto MAX_VERTICES = 65536;
static constexpr auto GL_SYNC_TIMEOUT = 500000;

struct RendererVertex
{
    glm::vec2 pos;      // Vertex coordinate (PSX Format)
    glm::vec3 color;    // Vertex RGB Color (PSX Format)
    glm::vec2 uv;       // Vertex Texture Coordinates (PSX Format)
};

struct DisplayState
{
    glm::ivec2  displayRes;
    glm::ivec4  displayArea;
    int         displayColorMode;
    bool        displayDisabled;
    bool        interlaced;
    bool        evenfield;

    bool operator==(const DisplayState& o) const
    {
        return memcmp(this, &o, sizeof(DisplayState)) == 0;
    }

};

struct RendererState
{
    glm::ivec4  drawingArea;
    glm::ivec2  drawingOffset;
    bool        drawingOnDisplayEnabled;
    glm::ivec2  texPageCoords;
    glm::ivec2  texMask;
    glm::ivec2  texOffset;
    glm::ivec2  clutTableCoords;
    int         texColorMode;
    bool        textured;
    bool        texBlending;
	bool		texDisable;
    bool		texRectangleXFlip;
	bool		texRectangleYFlip;
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
        
        //Set Output Window Size
        static void SetWindowsSize(int width, int height);

        //Set Video Display Status
        static void SetDisplayStart(lite::vec2t<uint16_t> displayStart);
        static void SetDisplayDisable(bool disabled);
        static void SetDisplayResolution(lite::vec2t<uint16_t> resolution);
        static void SetDisplayColorMode(uint8_t colorMode);
        static void SetDisplayInterlaceMode(bool interlaced);
        
        //Set Renderer Status
        static void SetDrawingArea(lite::vec4t<uint16_t> drawingArea);
        static void SetDrawingOffset(lite::vec2t<uint16_t> drawingOffset);
        static void SetDrawingOnDisplayEnabled(bool drawingOnDisplayEnable);
        static void SetTexturePage(lite::vec2t<uint16_t> texPageCoords);
        static void SetClutTable(lite::vec2t<uint16_t> clutCoords);
        static void SetTextureColorMode(uint8_t colorMode);
        static void SetTextureMode(bool textured, bool texBlending);
        static void SetTextureMask(lite::vec2t<uint8_t> textureMask);
        static void SetTextureOffset(lite::vec2t<uint8_t> textureOffset);
        static void SetTextureDisable(bool textureDisable);
        static void SetTransparency(bool semiTransparent);
        static void SetTransparencyMode(int semiTransparentMode);
        static void SetMaskBit(bool checkMask, bool forceMask);
        static void SetDither(bool dither);
		static void SetRectangleTextureFlip(bool flipX, bool flipY);
        
        //Draw Polygon Functions
        static void DrawPolygon(GpuVertex *vertex, uint16_t vertexNum);
        static void DrawRectangle(GpuVertex *vertex);
        static void DrawPoint(GpuVertex *vertex);
        static void DrawLine(GpuVertex *vertex);

        //VRAM Access Functions
		static bool CommitAccessBuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h);   //Copy Access Buffer to VRAM Texture
		static bool SyncAccessBuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h);     //Copy VRAM Texture to Access Buffer, to sync them before reading
        static bool WriteVRAM(uint16_t x, uint16_t y, uint16_t data);
        static bool WriteVRAMMasked(uint16_t x, uint16_t y, uint16_t data);
        static uint16_t ReadVRAM(uint16_t x, uint16_t y);
        static GLuint GetVRAMTextureObject();
        
 
    private:
        Renderer() {}

        void BeginBatch();
        bool FlushBatch();
        void PushVertex(const RendererVertex& v);
        void PushTriangle(const RendererVertex& v0, const RendererVertex& v1, const RendererVertex& v2);
        void PushQuad(const RendererVertex& v0, const RendererVertex& v1, const RendererVertex& v2, const RendererVertex& v3);
        void WaitForFence();
        void SetupRenderShader();
        void SetupFramebufferShader();
                
        //Frame Status
        bool                    frameReady;

        //Windows Size
        int                     windowWidth;                //Main Window Width - Set and Updated by psxemu
        int                     windowHeight;               //Main Window Height - Set and Updated by psxemu

        //Renderer State
        DisplayState            displayState;               //Current Display configuration
        RendererState           currentState;               //Current Rendering State being set
        RendererState           renderingState;             //Actual State used for rendering

        //Internal Structures
        GLuint                  vaoRenderBuffer;            //VAO for Rendering
        GLuint                  vboRenderBuffer;            //Vertex Persistent Buffer Object for Rendering
        GLuint                  vaoFrameBuffer;             //VAO for Framebuffer Quad
        GLuint                  vboFrameBuffer;             //Vertex Buffer Object for Framebuffer Quad
        GLuint                  vramTexture;                //Texture Object representing PSX VRAM
        GLuint                  vramAccessBufferID;         //VRAM Persistent Buffer Object
        GLuint                  vramFrameBuffer;            //Frame Buffer Object for Primitive Rendering
		GLuint                  vramDebugTexture;           //Texture Used to render VRAM content for debugging purposes on ImGui Widget

        //Persistent Vertex Buffer for Rendering Primitives
        RendererVertex*         mappedVertex = nullptr;     //Mapped Vertex Container, cointains the actual data.
        int                     vertexCount = 0;            //Number of Vertex on the current Batch
        GLsync                  fence = nullptr;            //OpenGL Sync object

        uint16_t*               vramAccessBuffer;           //Mapped PersiObject Container, contains the actual VRAM data

        std::unique_ptr<Shader> FramebufferShader;          //Framebuffer Rendering Shader Program   
        std::unique_ptr<Shader> RenderShader;               //Primitive Rendering Shader Program

        //Helpers
        static constexpr auto DEFAULT_HRES = 256.0f;
        static constexpr auto DEFAULT_VRES = 240.0f; 
};  

