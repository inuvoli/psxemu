#include "renderer.h"

Renderer::Renderer()
{
    pShader = new Shader("../src/shaders/vertexShader.glsl", "../src/shaders/fragmentShader.glsl");
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenVertexArrays(1, &VAO);
}

Renderer::~Renderer()
{
    delete pShader;
}
bool Renderer::updateDrawData()
{
    //Bind Vertex Array Object
    glBindVertexArray(VAO);
    //Bind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //Copy Vertices to binded Vertex Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
   
    //Bind Color Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    //Copy Vertices to binded Color Buffer Object
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
 
    //Set Attribute Pointer for Vertex Buffer Object
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(sizeof(GLfloat) * 3));

    //Enable Vertex Attrib array
    //glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);
    
    //Unbind Vertex Attrib Object
    glBindVertexArray(0);
    //Unbind Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, 0);

 
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINe);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    return true;

}

bool Renderer::renderFrame()
{
        //Use Shader
        pShader->Use();

        //Render Scene 
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

    return false;
}