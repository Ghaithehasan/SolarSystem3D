#include "ClockBox.h"
#include <vector>

ClockBox::ClockBox(float width, float height, float depth)
    : boxWidth(width), boxHeight(height), boxDepth(depth)
{
    GenerateBoxVertices();
}

ClockBox::~ClockBox() {
}

void ClockBox::GenerateBoxVertices() {
    float w = boxWidth / 2.0f;
    float h = boxHeight / 2.0f;
    float d = boxDepth / 2.0f;
    
    std::vector<float> vertices = {
        -w, -h,  d,    0.0f,  0.0f,  1.0f,    0.9f, 0.7f, 0.5f,  
         w, -h,  d,    0.0f,  0.0f,  1.0f,    0.9f, 0.7f, 0.5f,
         w,  h,  d,    0.0f,  0.0f,  1.0f,    0.9f, 0.7f, 0.5f,
        -w,  h,  d,    0.0f,  0.0f,  1.0f,    0.9f, 0.7f, 0.5f,
        
        -w, -h, -d,    0.0f,  0.0f, -1.0f,    0.5f, 0.3f, 0.2f,  
         w, -h, -d,    0.0f,  0.0f, -1.0f,    0.5f, 0.3f, 0.2f,
         w,  h, -d,    0.0f,  0.0f, -1.0f,    0.5f, 0.3f, 0.2f,
        -w,  h, -d,    0.0f,  0.0f, -1.0f,    0.5f, 0.3f, 0.2f,
        
        -w,  h,  d,   -1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
        -w,  h, -d,   -1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
        -w, -h, -d,   -1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
        -w, -h,  d,   -1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
        
         w,  h,  d,    1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
         w,  h, -d,    1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
         w, -h, -d,    1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
         w, -h,  d,    1.0f,  0.0f,  0.0f,    0.7f, 0.5f, 0.3f,
        
            -w,  h,  d,    0.0f,  1.0f,  0.0f,    1.0f, 0.8f, 0.6f,  
         w,  h,  d,    0.0f,  1.0f,  0.0f,    1.0f, 0.8f, 0.6f,
         w,  h, -d,    0.0f,  1.0f,  0.0f,    1.0f, 0.8f, 0.6f,
        -w,  h, -d,    0.0f,  1.0f,  0.0f,    1.0f, 0.8f, 0.6f,
        
        -w, -h,  d,    0.0f, -1.0f,  0.0f,    0.4f, 0.2f, 0.1f,  
         w, -h,  d,    0.0f, -1.0f,  0.0f,    0.4f, 0.2f, 0.1f,
         w, -h, -d,    0.0f, -1.0f,  0.0f,    0.4f, 0.2f, 0.1f,
        -w, -h, -d,    0.0f, -1.0f,  0.0f,    0.4f, 0.2f, 0.1f,
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,  2, 3, 0,
        4, 6, 5,  6, 4, 7,
        8, 9, 10,  10, 11, 8,
        12, 13, 14,  14, 15, 12,
        16, 17, 18,  18, 19, 16,
        20, 21, 22,  22, 23, 20
    };
    
    SetupBuffers(vertices.data(), vertices.size() * sizeof(float),
                indices.data(), indices.size());
}

void ClockBox::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();
    
    glm::mat4 model = GetModelMatrix();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    shader.setVec3("lightPos", glm::vec3(3.0f, 3.0f, 3.0f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

