#include "renderer.h"

Renderer::Renderer(const uint16_t* pData)
{
    //Init Pointer to GPU Video RAM
    videoRam = pData;

    //Compile Vertex & Fragment Shaders
    pShader = std::make_shared<Shader>("../src/gpu/shaders/vertexShader.glsl", "../src/gpu/shaders/fragmentShader.glsl");
    
    //Init Buffers
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &PBO);
    glGenTextures(1, &textureId);
    glGenVertexArrays(1, &VAO);
    
    //Alloc Persistent Pixel Buffer Object   
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);
	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512 * 2, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    //Alloc Texture on the GPU
    //Video Ram is a 1024x512 array of halfwords (16bit), each 16bit is a Texel. The content of those Texel
    //depends on the actual configuration of the GPU (CLUT, 4bit Texture, 8bit Texture ,...).
    //Video Ram Texels are passed to OpenGL as a buffer of 16bit RED values.
    //The actual content of each Texel (16bit) is decoded by the Fragment Shader 
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R16UI, 1024, 512, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
    /* Set the texture wrapping parameters. */
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	/* Set texture filtering parameters. */
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    //Get Pointer to Pixel Buffer Object
    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);
	textureData = (uint16_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512 * 2, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    //Define Vertex Attribs
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    //Vertex Array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribIPointer(0, 2, GL_INT, 0, (const GLvoid*)0);
    
    //Color Array
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glVertexAttribIPointer(1, 3, GL_UNSIGNED_INT, 0, (const GLvoid*)0);

    //Texuture Coordinates Array
    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glVertexAttribIPointer(2, 2, GL_UNSIGNED_INT, 0, (const GLvoid*)0);
    
    //Element Array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //Copy Elements to binded Element Buffer Object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(polygonIndices), polygonIndices, GL_DYNAMIC_DRAW);

    //Use & Init Shader
    pShader->Use();
    pShader->setUniformi("hRes", DEFAULT_HRES);
    pShader->setUniformi("vRes", DEFAULT_VRES);
    pShader->setUniformi("videoRam", 0);

   //Unbind all
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   
    //Turn On Wireframe Mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Renderer::~Renderer()
{
}

bool Renderer::DrawPolygon(uint8_t numVertex, const void* vertices, const void* colors, const void* texture, bool textured, uint16_t clutInfo, uint16_t texPageInfo)
{
    uint8_t numElements = 0;
     
    //Bind Vertex Address Object
    glBindVertexArray(VAO);  
    
    //Set Shader Uniforms
    pShader->setUniformi("textured", (textured) ? 1 : 0);
    pShader->setUniformui("clutInfo", clutInfo);
    pShader->setUniformui("texPageInfo", texPageInfo);
    
    //Bind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //Copy Vertices to binded Vertex Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * numVertex * 2, vertices, GL_DYNAMIC_DRAW);
    
    //Bind Color Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    //Copy Vertices to binded Color Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * numVertex * 3, colors, GL_DYNAMIC_DRAW);

    //Bind Texture Coordinates Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    //Copy Texture Coordinates to binded Color Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * numVertex * 2, texture, GL_DYNAMIC_DRAW);
    
    //Bind VideoRam as a Texture
    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R16UI, 1024, 512, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, (const void*)videoRam);

    //Render Polygon
    numElements = (numVertex == 3) ? 3 : 6;
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
   
    //Unbind Vertex Buffer Object
    glBindVertexArray(0);
    //Unbind Array Buffer 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Unbind VideoRam as a Texture
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    return true;
}

bool Renderer::DrawLine()
{
    //TODO
    return false;
}

bool Renderer::DrawRectangle()
{
    //TODO
    return false;
}

bool Renderer::SetResolution(int16_t hRes, int16_t vRes)
{
    pShader->setUniformi("hRes", hRes);
    pShader->setUniformi("vRes", vRes);

    return true;
}

bool Renderer::UpdateFrame()
{
    //Copy GPU VRAM content on the Pixel Buffer Array
    std::memcpy(textureData, videoRam, sizeof(uint16_t)*512*1024);
    //Empty Primitive Render Buffer
    drawData.clear();
    return true;
}