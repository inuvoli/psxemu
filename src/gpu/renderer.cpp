#include <loguru.hpp>
#include "renderer.h"
#include "debugger.h"

bool Renderer::Init()
{
    auto& r = instance();  // Alias al singleton

    //Init Internal Status
    r.frameReady = false;
    r.vertexCount = 0;

    //Init Video Status
    r.videoState.displayRes = { 256, 240 };
    r.videoState.displayArea = { 0, 0, 0, 0 };
    r.videoState.displayColorMode = 0;
    r.videoState.displayDisabled = true;
    r.videoState.interlaced = false;
    r.videoState.evenfield = false;

    //Init Rendering Status
    r.currentState.drawingArea = { 0, 0, 0, 0 };
    r.currentState.drawingOffset = { 0, 0 };
    r.currentState.drawingEnabled = false;
    r.currentState.texPageCoords = { 0,0 };
    r.currentState.texMask = { 0, 0 };
    r.currentState.texOffset = { 0, 0 };
    r.currentState.clutTableCoords = { 0, 0 };
    r.currentState.texColorMode = 0;
    r.currentState.textured = false;
    r.currentState.texBlending = false;
    r.currentState.semiTranparent = false;
    r.currentState.semiTransparentMode = 0;
    r.currentState.checkMask = false;
    r.currentState.forceMask = false;
    r.currentState.dither = false;

    //Quad for Video Rendering
    float quad[] =
    {
    -1.0, -1.0, 0.0, 1.0,
     1.0, -1.0, 1.0, 1.0,
     1.0,  1.0, 1.0, 0.0,
    -1.0,  1.0, 0.0, 0.0
    };

    //Compile Vertex & Fragment Shaders
    try
    {
        r.RenderShader = std::make_unique<Shader>("../src/gpu/shaders/vertexRenderShader.glsl", "../src/gpu/shaders/fragmentRenderShader.glsl");
        r.FramebufferShader = std::make_unique<Shader>("../src/gpu/shaders/vertexFramebufferShader.glsl", "../src/gpu/shaders/fragmentFramebufferShader.glsl");
    }
    catch (const std::exception& e)
    {
        LOG_F(ERROR, "RND - Shader creation failed: %s", e.what());
        return false;
    }

    LOG_F(INFO, "RND - Shaders Compiled");

    //Create Texture representing PSX VRAM
    glCreateTextures(GL_TEXTURE_2D, 1, &r.vramTexture);
    if (r.vramTexture == 0)
    {
        LOG_F(ERROR, "RND - glCreateTextures returned texture 0");
        return false;
    }
    glTextureStorage2D(r.vramTexture, 1, GL_R16UI, 1024, 512);
    glTextureParameteri(r.vramTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(r.vramTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(r.vramTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(r.vramTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
    // Create Persistent Mapped Array which rappresent PSX VRAM
    glCreateBuffers(1, &r.vramBuffer);
    glNamedBufferStorage(r.vramBuffer, 1024*512*sizeof(uint16_t), nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    r.mappedVRAM = (uint16_t*)glMapNamedBufferRange(r.vramBuffer, 0, 1024*512*sizeof(uint16_t), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    if (r.mappedVRAM == nullptr)
    {
        LOG_F(ERROR, "RND - glMapNamedBufferRange failed for vramBuffer");
        return false;
    }

    //Create Framebuffer Object for VRAM Texture, Shader renders to this FBO to update VRAM Texture
    glCreateFramebuffers(1, &r.vramFrameBuffer);
    glNamedFramebufferTexture(r.vramFrameBuffer, GL_COLOR_ATTACHMENT0, r.vramTexture, 0);
    GLenum fbStatus = glCheckNamedFramebufferStatus(r.vramFrameBuffer, GL_FRAMEBUFFER);
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_F(ERROR, "RND - VRAM framebuffer incomplete: 0x%X", fbStatus);
        return false;
    }

    //Create Vertex Array Object & Vertex Buffer Object for Video Output Quad
    glCreateVertexArrays(1, &r.vaoFrameBuffer);
    glCreateBuffers(1, &r.vboFrameBuffer);
    glNamedBufferData(r.vboFrameBuffer, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(r.vaoFrameBuffer, 0, r.vboFrameBuffer, 0, 4 * sizeof(float));
    
    glEnableVertexArrayAttrib(r.vaoFrameBuffer, 0);
    glEnableVertexArrayAttrib(r.vaoFrameBuffer, 1);
    glVertexArrayAttribFormat(r.vaoFrameBuffer, 0, 2, GL_FLOAT, false, 0);
    glVertexArrayAttribFormat(r.vaoFrameBuffer, 1, 2, GL_FLOAT, false, 2 * sizeof(float));
    glVertexArrayAttribBinding(r.vaoFrameBuffer, 0, 0);
    glVertexArrayAttribBinding(r.vaoFrameBuffer, 1, 0);

    //Alloc Persistent Render Buffer for Primitive Drawing
    glCreateVertexArrays(1, &r.vaoRenderBuffer);
    glCreateBuffers(1, &r.vboRenderBuffer);
    glNamedBufferStorage(r.vboRenderBuffer, MAX_VERTICES * sizeof(RendererVertex), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    r.mappedVertex = (RendererVertex*)glMapNamedBufferRange(r.vboRenderBuffer, 0, MAX_VERTICES * sizeof(RendererVertex), GL_MAP_WRITE_BIT |GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    if (r.mappedVertex == nullptr)
    {
        LOG_F(ERROR, "RND - glMapNamedBufferRange failed for vboRenderBuffer");
        return false;
    }
    glVertexArrayVertexBuffer(r.vaoRenderBuffer, 0, r.vboRenderBuffer, 0, sizeof(RendererVertex));

    glEnableVertexArrayAttrib(r.vaoRenderBuffer, 0);
    glEnableVertexArrayAttrib(r.vaoRenderBuffer, 1);
    glEnableVertexArrayAttrib(r.vaoRenderBuffer, 2);
    glVertexArrayAttribFormat(r.vaoRenderBuffer, 0, 2, GL_FLOAT, false, offsetof(RendererVertex, pos));
    glVertexArrayAttribFormat(r.vaoRenderBuffer, 1, 3, GL_FLOAT, false, offsetof(RendererVertex, color));
    glVertexArrayAttribFormat(r.vaoRenderBuffer, 2, 2, GL_FLOAT, false, offsetof(RendererVertex, uv));
    glVertexArrayAttribBinding(r.vaoRenderBuffer, 0, 0);
    glVertexArrayAttribBinding(r.vaoRenderBuffer, 1, 0);
    glVertexArrayAttribBinding(r.vaoRenderBuffer, 2, 0);

#ifdef DEBUGGER_ENABLED
    glCreateTextures(GL_TEXTURE_2D, 1, &r.vramDebugTexture);
    //glTextureStorage2D(r.vramDebugTexture, 1, GL_RGBA8, 1024, 512);
    glTextureStorage2D(r.vramDebugTexture, 1, GL_RGB5_A1, 1024, 512);
    glTextureParameteri(r.vramDebugTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(r.vramDebugTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
     
    //Turn On Wireframe Mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    return true;
}

bool Renderer::Reset()
{
    auto& r = instance();  // Alias al singleton

    //Init Internal Status
    r.frameReady = false;
    r.vertexCount = 0;

    //Init Video Status
    r.videoState.displayRes = { 256, 240 };
    r.videoState.displayArea = { 0, 0, 0, 0 };
    r.videoState.displayColorMode = 0;
    r.videoState.displayDisabled = true;
    r.videoState.interlaced = false;
    r.videoState.evenfield = false;

    //Init Rendering Status
    r.currentState.drawingArea = { 0, 0, 0, 0 };
    r.currentState.drawingOffset = { 0, 0 };
    r.currentState.drawingEnabled = false;
    r.currentState.texPageCoords = { 0,0 };
    r.currentState.texMask = { 0, 0 };
    r.currentState.texOffset = { 0, 0 };
    r.currentState.clutTableCoords = { 0, 0 };
    r.currentState.texColorMode = 0;
    r.currentState.textured = false;
    r.currentState.texBlending = false;
    r.currentState.semiTranparent = false;
    r.currentState.semiTransparentMode = 0;
    r.currentState.checkMask = false;
    r.currentState.forceMask = false;
    r.currentState.dither = false;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------
//
// Renderer Drawing to Screen Functions
//
//--------------------------------------------------------------------------------------------------------------------
bool Renderer::FrameReady()
{   
    auto& r = instance();  // Alias al singleton      

    bool status = r.frameReady;

    if (status)
        r.frameReady = false;

    return status;
}

void Renderer::vBlank()
{
    auto& r = instance();  // Alias al singleton

    r.frameReady = true;   
}

void Renderer::ScreenUpdate()
{
    auto& r = instance();  // Alias al singleton      
    
    //Flush Remaining Vertex to VRAM before rendering to Video
    r.FlushBatch();

    if (!r.videoState.displayDisabled)
    {
        //Setup Framebuffer Shader
        r.SetupFramebufferShader();  

        //Render to Screen
        glBindVertexArray(r.vaoFrameBuffer);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

#ifdef DEBUGGER_ENABLED
        r.UpdateDebugTexture();
#endif

        LOG_F(2, "RND - Video Display Rendered!");
    }
}

void Renderer::UpdateDebugTexture()
{
#ifdef DEBUGGER_ENABLED
    auto& r = instance();  // Alias al singleton    

    // Read from rendered vramTexture and upload to debug texture
    std::vector<uint16_t> vramData(1024 * 512);
    glGetTextureImage(r.vramTexture, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, vramData.size() * sizeof(uint16_t), vramData.data());

    // Swap red (bits 0-4) and blue (bits 10-14) for GL_RGB5_A1
    for (auto& pixel : vramData)
    {
        uint16_t r5 = (pixel >> 0) & 31;
        uint16_t g5 = (pixel >> 5) & 31;
        uint16_t b5 = (pixel >> 10) & 31;
    //    uint16_t mask = (pixel >> 15) & 1;
        uint16_t mask = 1;

       pixel = (mask << 0) | (b5 << 5) | (g5 << 10) | (r5 << 15);
    }

    glTextureSubImage2D(r.vramDebugTexture, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, vramData.data());
#endif
}

//--------------------------------------------------------------------------------------------------------------------
//
// Primitive Drawing Functions, converts PSX Primitives to Triangles
//
//--------------------------------------------------------------------------------------------------------------------
void Renderer::PushVertex(const RendererVertex& v)
{
    auto& r = instance();  // Alias al singleton

    if (r.mappedVertex == nullptr)
    {
        LOG_F(ERROR, "RND - mappedVertex is null in PushVertex");
        return;
    }

    r.mappedVertex[r.vertexCount++] = v;

    if (r.vertexCount >= MAX_VERTICES)
        r.FlushBatch();
}

void Renderer::DrawPolygon(GpuVertex *vertex, uint16_t vertexNum)
{
    auto& r = instance();  // Alias al singleton

    RendererVertex v[4];
    
    r.BeginBatch();

    for (int i = 0; i < vertexNum; i++)
    {
        v[i].pos = glm::vec2((float)vertex[i].x, (float)vertex[i].y);
        v[i].uv = glm::vec2((float)vertex[i].u, (float)vertex[i].v);
        v[i].color = glm::vec3((float)vertex[i].r, (float)vertex[i].g, (float)vertex[i].b);    
    }

    if (vertexNum == 3)
    {
        //Triangle
        r.PushVertex(v[0]);
        r.PushVertex(v[1]);
        r.PushVertex(v[2]);
        LOG_F(2, "RND - Draw POLYGON (3 Vertex)");
        LOG_F(2, "RND - Vertex 1 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[0].pos.x, v[0].pos.y, v[0].color.x, v[0].color.y, v[0].color.z, v[0].uv.x, v[0].uv.y);
        LOG_F(2, "RND - Vertex 2 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[1].pos.x, v[1].pos.y, v[1].color.x, v[1].color.y, v[1].color.z, v[1].uv.x, v[1].uv.y);
        LOG_F(2, "RND - Vertex 3 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[2].pos.x, v[2].pos.y, v[2].color.x, v[2].color.y, v[2].color.z, v[2].uv.x, v[2].uv.y);
    }
    else if (vertexNum == 4)
    {
        //Quad splitted in two triangles
        r.PushVertex(v[0]);
        r.PushVertex(v[1]);
        r.PushVertex(v[2]);

        r.PushVertex(v[1]);
        r.PushVertex(v[2]);
        r.PushVertex(v[3]);

        LOG_F(2, "RND - Draw POLYGON (4 Vertex)");
        LOG_F(2, "RND - Vertex 1 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[0].pos.x, v[0].pos.y, v[0].color.x, v[0].color.y, v[0].color.z, v[0].uv.x, v[0].uv.y);
        LOG_F(2, "RND - Vertex 2 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[1].pos.x, v[1].pos.y, v[1].color.x, v[1].color.y, v[1].color.z, v[1].uv.x, v[1].uv.y);
        LOG_F(2, "RND - Vertex 3 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[2].pos.x, v[2].pos.y, v[2].color.x, v[2].color.y, v[2].color.z, v[2].uv.x, v[2].uv.y);
        LOG_F(2, "RND - Vertex 4 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[3].pos.x, v[3].pos.y, v[3].color.x, v[3].color.y, v[3].color.z, v[3].uv.x, v[3].uv.y);
    }
}

void Renderer::DrawRectangle(GpuVertex *vertex)
{
    auto& r = instance();  // Alias al singleton

    RendererVertex v[4];
    
    r.BeginBatch();

    for (int i = 0; i < 4; i++)
    {
        v[i].pos = glm::vec2((float)vertex[i].x, (float)vertex[i].y);
        v[i].uv = glm::vec2((float)vertex[i].u, (float)vertex[i].v);
        v[i].color = glm::vec3((float)vertex[i].r, (float)vertex[i].g, (float)vertex[i].b);    
    }
   
    //Quad splitted in two triangles
    r.PushVertex(v[0]);
    r.PushVertex(v[1]);
    r.PushVertex(v[2]);

    r.PushVertex(v[1]);
    r.PushVertex(v[2]);
    r.PushVertex(v[3]);

    LOG_F(2, "RND - Draw RECTANGLE");
    LOG_F(2, "RND - Vertex 1 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[0].pos.x, v[0].pos.y, v[0].color.x, v[0].color.y, v[0].color.z, v[0].uv.x, v[0].uv.y);
    LOG_F(2, "RND - Vertex 2 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[1].pos.x, v[1].pos.y, v[1].color.x, v[1].color.y, v[1].color.z, v[1].uv.x, v[1].uv.y);
    LOG_F(2, "RND - Vertex 3 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[2].pos.x, v[2].pos.y, v[2].color.x, v[2].color.y, v[2].color.z, v[2].uv.x, v[2].uv.y);
    LOG_F(2, "RND - Vertex 4 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[3].pos.x, v[3].pos.y, v[3].color.x, v[3].color.y, v[3].color.z, v[3].uv.x, v[3].uv.y);
}

void Renderer::DrawPoint(GpuVertex *vertex)
{
    auto& r = instance();  // Alias al singleton

    RendererVertex v[3];

    r.BeginBatch();
        
    v[0].pos = glm::vec2((float)vertex[0].x, (float)vertex[0].y);
    v[0].uv = glm::vec2((float)vertex[0].u, (float)vertex[0].v);
    v[0].color = glm::vec3((float)vertex[0].r, (float)vertex[0].g, (float)vertex[0].b);

    v[1].pos = glm::vec2((float)vertex[0].x + 1.0f, (float)vertex[0].y);
    v[1].uv = glm::vec2((float)vertex[1].u, (float)vertex[1].v);
    v[1].color = glm::vec3((float)vertex[1].r, (float)vertex[1].g, (float)vertex[1].b);

    v[2].pos = glm::vec2((float)vertex[0].x, (float)vertex[0].y + 1.0f);
    v[2].uv = glm::vec2((float)vertex[2].u, (float)vertex[2].v);
    v[2].color = glm::vec3((float)vertex[2].r, (float)vertex[2].g, (float)vertex[2].b);    

    //Create a small triangle to represent the point
    r.PushVertex(v[0]);
    r.PushVertex(v[1]);
    r.PushVertex(v[2]);

    LOG_F(2, "RND - Draw POINT");
    LOG_F(2, "RND - Vertex 1 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[0].pos.x, v[0].pos.y, v[0].color.x, v[0].color.y, v[0].color.z, v[0].uv.x, v[0].uv.y);
    LOG_F(2, "RND - Vertex 2 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[1].pos.x, v[1].pos.y, v[1].color.x, v[1].color.y, v[1].color.z, v[1].uv.x, v[1].uv.y);
    LOG_F(2, "RND - Vertex 3 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[2].pos.x, v[2].pos.y, v[2].color.x, v[2].color.y, v[2].color.z, v[2].uv.x, v[2].uv.y);
}

void Renderer::DrawLine(GpuVertex *vertex)
{
    auto& r = instance();  // Alias al singleton

    RendererVertex v[4];
    glm::vec2 p0, p1, d, n, dir;
    
    r.BeginBatch();

    p0 = glm::floor(glm::vec2((float)vertex[0].x, (float)vertex[0].y));
    p1 = glm::floor(glm::vec2((float)vertex[1].x, (float)vertex[1].y));
    
    d = p1 - p0;
    if (glm::length(d) == 0)
        return;

    dir = glm::normalize(d);
    n = glm::vec2(-dir.y, dir.x) * 0.5f;

    v[0].pos = p0 + n;
    v[0].uv = glm::vec2((float)vertex[0].u, (float)vertex[0].v);
    v[0].color = glm::vec3((float)vertex[0].r, (float)vertex[0].g, (float)vertex[0].b);

    v[1].pos = p1 + n;
    v[1].uv = glm::vec2((float)vertex[1].u, (float)vertex[1].v);
    v[1].color = glm::vec3((float)vertex[1].r, (float)vertex[1].g, (float)vertex[1].b);

    v[2].pos = p1 - n;
    v[2].uv = glm::vec2((float)vertex[1].u, (float)vertex[1].v);
    v[2].color = glm::vec3((float)vertex[1].r, (float)vertex[1].g, (float)vertex[1].b);

    v[3].pos = p0 - n;
    v[3].uv = glm::vec2((float)vertex[0].u, (float)vertex[0].v);
    v[3].color = glm::vec3((float)vertex[0].r, (float)vertex[0].g, (float)vertex[0].b);

    //Quad splitted in two triangles
    r.PushVertex(v[0]);
    r.PushVertex(v[1]);
    r.PushVertex(v[2]);

    r.PushVertex(v[2]);
    r.PushVertex(v[3]);
    r.PushVertex(v[0]);

    LOG_F(2, "RND - Draw LINE");
    LOG_F(2, "RND - Vertex 1 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[0].pos.x, v[0].pos.y, v[0].color.x, v[0].color.y, v[0].color.z, v[0].uv.x, v[0].uv.y);
    LOG_F(2, "RND - Vertex 2 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[1].pos.x, v[1].pos.y, v[1].color.x, v[1].color.y, v[1].color.z, v[1].uv.x, v[1].uv.y);
    LOG_F(2, "RND - Vertex 3 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[2].pos.x, v[2].pos.y, v[2].color.x, v[2].color.y, v[2].color.z, v[2].uv.x, v[2].uv.y);
    LOG_F(2, "RND - Vertex 4 - Pos: [%f, %f], Color: [%f, %f, %f], UV: [%f, %f]", v[3].pos.x, v[3].pos.y, v[3].color.x, v[3].color.y, v[3].color.z, v[3].uv.x, v[3].uv.y);
}

//--------------------------------------------------------------------------------------------------------------------
//
// VRAM Management Functions
//
//--------------------------------------------------------------------------------------------------------------------
bool Renderer::CommitVRAMWrite()
{
    auto& r = instance();  // Alias al singleton
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, r.vramBuffer);
    glTextureSubImage2D(r.vramTexture, 0, 0, 0, 1024, 512, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    LOG_F(2, "RND - VRAM Write Committed!");
    return true;
}

bool Renderer::WriteVRAM(uint16_t x, uint16_t y, uint16_t data)
{
    auto& r = instance();  // Alias al singleton

    if (r.mappedVRAM == nullptr)
    {
        LOG_F(ERROR, "RND mappedVRAM is null in WriteVRAM");
        return false;
    }
    r.mappedVRAM[y * 1024 + x] = data;
    return true;
}

uint16_t Renderer::ReadVRAM(uint16_t x, uint16_t y)
{
    auto& r = instance();  // Alias al singleton

    if (r.mappedVRAM == nullptr)
    {
        LOG_F(ERROR, "RND - mappedVRAM is null in ReadVRAM");
        return 0;
    }

    return r.mappedVRAM[y * 1024 + x];
}

GLuint Renderer::GetVRAMTextureObject()
{
    auto& r = instance();  // Alias al singleton
    return r.vramDebugTexture;
}

//--------------------------------------------------------------------------------------------------------------------
//
// Batch Rendering Management Functions
//
//--------------------------------------------------------------------------------------------------------------------
void Renderer::BeginBatch()
{
    auto& r = instance();  // Alias al singleton

    //Check if current rendering state is different from the last one used for rendering
    if (!(r.currentState == r.renderingState))
    {
        //Update Rendering State
        r.FlushBatch();
        r.renderingState = r.currentState;

        LOG_F(2, "RND - Begin new Vertex Batch!");
    }
}

void Renderer::FlushBatch()
{
    auto& r = instance();  // Alias al singleton
    
    //Skip if there's no vertex to flush
    if (r.vertexCount == 0)
        return;

    //Wait for previous command to end
    if (r.fence)
    {
        while (glClientWaitSync(r.fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1) == GL_TIMEOUT_EXPIRED) {}
        glDeleteSync(r.fence);
        r.fence = nullptr;
    }

    //Initialize Render Shader according to renderingState
    r.SetupRenderShader();

    //Draw to FrameBuffer
    glBindVertexArray(r.vaoRenderBuffer);
    glDrawArrays(GL_TRIANGLES, 0, r.vertexCount);

    LOG_F(2, "RND - Flushed Vertex Batch (VertexNum: %d)", r.vertexCount);

    r.fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    r.vertexCount = 0;
}

void Renderer::SetupRenderShader()
{
    auto& r = instance();  // Alias al singleton
    
    glBindFramebuffer(GL_FRAMEBUFFER, r.vramFrameBuffer);
    glViewport(0, 0, 1024, 512);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    r.RenderShader->Use();

    //Set Drawing Area and Drawing Offset for Vertex Shader
    r.RenderShader->setUniformi("uDrawArea", r.renderingState.drawingArea);
    r.RenderShader->setUniformi("uDrawOffset", r.renderingState.drawingOffset);
    r.RenderShader->setUniformb("uDrawEnable", r.renderingState.drawingEnabled);

    //Bind VRAM Texture, Shader reads PSX CLUT and TEXPAGE value from this Texture
    glBindTextureUnit(0, r.vramTexture); 
    r.RenderShader->setUniformi("uVRAM", 0);

    //Set CLUT and TEXPAGE parameters
    r.RenderShader->setUniformi("uTPage", r.renderingState.texPageCoords);
    r.RenderShader->setUniformi("uClut", r.renderingState.clutTableCoords);
    r.RenderShader->setUniformi("uTexColorMode", r.renderingState.texColorMode);
    r.RenderShader->setUniformi("uTexMask", r.renderingState.texMask);
    r.RenderShader->setUniformi("uTexOffset", r.renderingState.texOffset);

    //Set Texture Flags
    r.RenderShader->setUniformb("uTextured", r.renderingState.textured);
    r.RenderShader->setUniformb("uTexBlending", r.renderingState.texBlending);
    
    //Set Semi Transparency Flags and Mode
    r.RenderShader->setUniformb("uSemiTrans", r.renderingState.semiTranparent);
    r.RenderShader->setUniformi("uSemiTransMode", r.renderingState.semiTransparentMode);

    //Set Mask bit Flags
    r.RenderShader->setUniformb("uCheckMaskBit", r.renderingState.checkMask);
    r.RenderShader->setUniformb("uForceMaskBit", r.renderingState.forceMask);

    //Set Dither Flag
    r.RenderShader->setUniformb("uDither", r.renderingState.dither);
}

void Renderer::SetupFramebufferShader()
{
    auto& r = instance();  // Alias al singleton

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, r.videoState.displayRes.x, r.videoState.displayRes.y);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    r.FramebufferShader->Use();

    //Bind FrameBuffer Texture, Shader reads from this Texture according to DisplayArea and output to Screen
    glBindTextureUnit(0, r.vramTexture);
    r.FramebufferShader->setUniformui("uVRAM", 0);

    //Set Display Area parameters
    r.FramebufferShader->setUniformi("uDisplayArea", r.videoState.displayArea);

    //Set Display Color Mode
    r.FramebufferShader->setUniformi("uDisplayColorMode", r.videoState.displayColorMode);

    //Set Display Interlace mode parameters
    r.FramebufferShader->setUniformb("uInterlaced", r.videoState.interlaced);
    r.FramebufferShader->setUniformb("uEvenField", r.videoState.evenfield);

    //Evenfield keep toogling every frame, it is used by the Shader onlu if interlaced in set
    r.videoState.evenfield = !r.videoState.evenfield;
}

//--------------------------------------------------------------------------------------------------------------------
//
// Video Display State Setting Functions
//
//--------------------------------------------------------------------------------------------------------------------
void Renderer::SetDisplayDisable(bool disabled)
{
    auto& r = instance();  // Alias al singleton
    r.videoState.displayDisabled = disabled;
}

void Renderer::SetDisplayArea(lite::vec4t<uint16_t> displayArea)
{
    auto& r = instance();  // Alias al singleton
    r.videoState.displayArea.x = (GLint)displayArea.x; //Starting X Coordinate
    r.videoState.displayArea.y = (GLint)displayArea.y; //Starging Y Coordinate
    r.videoState.displayArea.z = (GLint)displayArea.z; //Width
    r.videoState.displayArea.w = (GLint)displayArea.w; //Height
}

void Renderer::SetDisplayResolution(lite::vec2t<uint16_t> resolution)
{
    auto& r = instance();  // Alias al singleton
    r.videoState.displayRes.x = (GLint)resolution.x;
    r.videoState.displayRes.y = (GLint)resolution.y;
}

void Renderer::SetDisplayColorMode(uint8_t colorMode)
{
    auto& r = instance();  // Alias al singleton
    r.videoState.displayColorMode = (GLint)colorMode;
}

void Renderer::SetDisplayInterlaceMode(bool interlaced)
{
    auto& r = instance();  // Alias al singleton
    r.videoState.interlaced = interlaced;
}

//--------------------------------------------------------------------------------------------------------------------
//
// Renderer State Setting Functions
//
//--------------------------------------------------------------------------------------------------------------------
void Renderer::SetDrawingArea(lite::vec4t<uint16_t> drawingArea)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.drawingArea.x = (GLuint)drawingArea.x;
    r.currentState.drawingArea.y = (GLuint)drawingArea.y;
    r.currentState.drawingArea.z = (GLuint)drawingArea.z;
    r.currentState.drawingArea.w = (GLuint)drawingArea.w;
}

void Renderer::SetDrawingOffset(lite::vec2t<uint16_t> drawingOffset)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.drawingOffset.x = (GLuint)drawingOffset.x;
    r.currentState.drawingOffset.y = (GLuint)drawingOffset.y;
}

void Renderer::SetDrawingEnabled(bool drawingEnable)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.drawingEnabled = drawingEnable;
}

void Renderer::SetTransparency(bool semiTransparent)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.semiTranparent = semiTransparent;
}

void Renderer::SetTransparencyMode(int semiTransparentMode)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.semiTransparentMode = semiTransparentMode;
}

void Renderer::SetMaskBit(bool checkMask, bool forceMask)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.checkMask = checkMask;
    r.currentState.forceMask = forceMask;
}

void Renderer::SetDither(bool dither)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.dither = dither;
}

void Renderer::SetTextureMode(bool textured, bool texBlending)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.textured = textured;
    r.currentState.texBlending = texBlending;
}

void Renderer::SetTextureMask(lite::vec2t<uint8_t> textureMask)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.texMask.x = (GLint)textureMask.x;
    r.currentState.texMask.y = (GLint)textureMask.y;
}

void Renderer::SetTextureOffset(lite::vec2t<uint8_t> textureOffset)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.texOffset.x = (GLint)textureOffset.x;
    r.currentState.texOffset.y = (GLint)textureOffset.y;
}

void Renderer::SetTexturePage(lite::vec2t<uint16_t> texPageCoords)
{   
    auto& r = instance();  // Alias al singleton
    r.currentState.texPageCoords.x = (GLint)texPageCoords.x;
    r.currentState.texPageCoords.y = (GLint)texPageCoords.y;
}

void Renderer::SetClutTable(lite::vec2t<uint16_t> clutCoords)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.clutTableCoords.x = (GLint)clutCoords.x;
    r.currentState.clutTableCoords.y = (GLint)clutCoords.y;
}

void Renderer::SetTextureColorMode(uint8_t colorMode)
{
    auto& r = instance();  // Alias al singleton
    r.currentState.texColorMode = (int)colorMode;
}
