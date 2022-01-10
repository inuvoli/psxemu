#include "renderer.h"

Renderer::Renderer()
{
    pShader = new Shader("../src/shaders/vertexShader.glsl", "../src/shaders/fragmentShader.glsl");
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenVertexArrays(1, &VAO);

    //Use Shader
    pShader->Use();
    pShader->setUniformi("hRes", 640);
    pShader->setUniformi("vRes", 480);

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
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Turn On Wireframe Mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Renderer::~Renderer()
{
    delete pShader;
}
bool Renderer::updateDrawData()
{
    //Bind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //Copy Vertices to binded Vertex Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    //Bind Color Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    //Copy Vertices to binded Color Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
    //Unbind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, 0);
 
    return true;

}

bool Renderer::renderFrame()
{
         //Render Scene 
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

    return false;
}