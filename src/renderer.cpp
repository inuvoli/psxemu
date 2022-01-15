#include "renderer.h"

Renderer::Renderer()
{
    pShader = new Shader("../src/shaders/vertexShader.glsl", "../src/shaders/fragmentShader.glsl");
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    //Use Shader
    pShader->Use();
    pShader->setUniformi("hRes", DEFAULT_HRES);
    pShader->setUniformi("vRes", DEFAULT_VRES);

    //Define Vertex Attribs
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //Vertex Array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribIPointer(0, 2, GL_INT, 0, (const GLvoid*)0);
    
    //Color Array
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glVertexAttribIPointer(1, 3, GL_UNSIGNED_INT, 0, (const GLvoid*)0); 
    
    //Element Array
    glBindBuffer(GL_ARRAY_BUFFER, EBO);
   
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
   
    //Turn On Wireframe Mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Renderer::~Renderer()
{
    delete pShader;
}

bool Renderer::renderPolygon(uint8_t numVertex, const void* vertices, const void* colors)
{
    uint8_t numElements = 0;
    GLuint indices[] = {0, 1, 2, 1, 2, 3};
     
    //Bind Vertex Address Object
    glBindVertexArray(VAO);  
    //Bind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //Copy Vertices to binded Vertex Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * numVertex * 2, vertices, GL_DYNAMIC_DRAW);
    //Bind Color Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    //Copy Vertices to binded Color Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * numVertex * 3, colors, GL_DYNAMIC_DRAW);
    //Bind Element Buffer Object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //Copy Elements to binded Element Buffer Object
    numElements = (numVertex == 3) ? 3 : 6;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
    //Render Polygon
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
    //Unbind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

bool Renderer::renderLine()
{
    //TODO
    return false;
}

bool Renderer::renderRectangle()
{
    //TODO
    return false;
}

bool Renderer::rendererSetResolution(int16_t hRes, int16_t vRes)
{
    pShader->setUniformi("hRes", hRes);
    pShader->setUniformi("vRes", vRes);

    return true;
}