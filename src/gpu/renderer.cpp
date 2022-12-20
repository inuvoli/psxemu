#include "renderer.h"

Renderer::Renderer(const uint16_t* pData)
{
    //Init internal variables
    drawDataVertexNumber = 0;

    //Init Pointer to GPU Video RAM
    videoRam = pData;

    //Compile Vertex & Fragment Shaders
    pShader = std::make_shared<Shader>("../src/gpu/shaders/vertexShader.glsl", "../src/gpu/shaders/fragmentShader.glsl");
    //Use & Init Shader
     pShader->Use();
     pShader->setUniformf("hRes", DEFAULT_HRES);
     pShader->setUniformf("vRes", DEFAULT_VRES);
     pShader->setUniformi("videoRam", 0);    
    
    //Init Buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &PBO);
    glGenTextures(1, &textureId);

    //Define Vertex Attribs
    glBindVertexArray(VAO); //----------------------------------------------------------------------------------------------------------

    //Alloc Vertex Array Space
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexInfo) * maxVertices, nullptr, GL_DYNAMIC_DRAW);

    //Define Vertex Attributes Info
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, vertexColor));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, vertexPosition));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, vertexTexCoords));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, clutTableCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, textured));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, texPageCoords));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, texColorDepth));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, texBlending));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, transparent));
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (GLvoid*)offsetof(VertexInfo, transMode));
    glEnableVertexAttribArray(9);
    
   //Unbind all
    glBindVertexArray(0); //----------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Alloc Persistent Pixel Buffer Object   
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);
	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512 * 2, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
    //Video Ram is a 1024x512 array of halfwords (16bit), each 16bit is a Texel. The content of those Texel
    //depends on the actual configuration of the GPU (CLUT, 4bit Texture, 8bit Texture ,...).
    //Video Ram Texels are passed to OpenGL as a buffer of 16bit RED values.
    //The actual content of each Texel (16bit) is decoded by the Fragment Shader 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
    //Set the texture wrapping parameters.
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//Set texture filtering parameters.
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //Alloc Texture Memory on the GPU
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R16UI, 1024, 512, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    //Get Pointer to Pixel Buffer Object
    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);
    textureData = (uint16_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512 * 2, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    //Alloc Vertex Vector
    drawData.reserve(maxVertices);
       
    //Turn On Wireframe Mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Renderer::~Renderer()
{
    //TODO
}

bool Renderer::reset()
{
    drawData.clear();
    drawDataVertexNumber = 0;

    return true;
}

bool Renderer::SetResolution(int16_t hRes, int16_t vRes)
{
    pShader->setUniformf("hRes", (GLfloat)hRes);
    pShader->setUniformf("vRes", (GLfloat)vRes);

    return true;
}

bool Renderer::InsertPolygon(std::vector<VertexInfo>& e)
{
    //Push all Vertex Info data associated with the actual polygon on the Renderer Draw Buffer
    drawData.insert(drawData.end(), e.begin(), e.end());

    return true;
}

bool Renderer::InsertLine()
{
    //TODO
    return false;
}

bool Renderer::InsertRectangle()
{
    //TODO
    return false;
}

bool Renderer::NewFrameReady()
{
    //Skip if no vertex is available
    if (drawData.size() == 0) return false;

    //Copy GPU VRAM content on the Pixel Buffer Array to access PSX Textures Pages
    std::memcpy(textureData, videoRam, sizeof(uint16_t)*512*1024);
    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 1024, 512, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    //Update Vertex Array Buffer with current content of drawData.
    //Bind Vertex Array Object
    glBindVertexArray(VAO);    
    //Bind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //Copy all Vertex Vector Data
    glBufferSubData(GL_ARRAY_BUFFER, 0, drawData.size() * sizeof(VertexInfo), (GLvoid*)drawData.data());
    //Unbind Vertex Array Object
    glBindVertexArray(0);
    //Unbind Array Buffer 
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Store Number the number of Vertex copied on the Vertex Buffer Object and Empty DrawData Structure
    drawDataVertexNumber = (int)drawData.size();
    drawData.clear();

    return true;
}

bool Renderer::RenderDrawData()
{
    //Bind Vertex Address Object
    glBindVertexArray(VAO);    
    //Bind VideoRam as a Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
    //Draw all Triangles
    glDrawArrays(GL_TRIANGLES, 0, drawDataVertexNumber);
    //Unbind Vertex Buffer Object
    glBindVertexArray(0);
    //Unbind VideoRam as a Texture
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    return true;
}