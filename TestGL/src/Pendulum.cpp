#include "Pendulum.h"
#include <vector>
#include <cmath>

Pendulum::Pendulum(float length, float bobRadius)
    : pendulumLength(length), bobRadius(bobRadius),
      amplitude(0.5f), period(2.0f), currentTime(0.0f), isMoving(true),
      rodVAO(0), rodVBO(0), rodEBO(0),
      bobVAO(0), bobVBO(0), bobEBO(0),
      rodIndexCount(0), bobIndexCount(0)
{
    GenerateRodVertices();
    GenerateBobVertices();
    SetPosition(glm::vec3(0.0f, -0.5f, 0.0f));  
}

Pendulum::~Pendulum() {
    if (rodVAO != 0) glDeleteVertexArrays(1, &rodVAO);
    if (rodVBO != 0) glDeleteBuffers(1, &rodVBO);
    if (rodEBO != 0) glDeleteBuffers(1, &rodEBO);
    if (bobVAO != 0) glDeleteVertexArrays(1, &bobVAO);
    if (bobVBO != 0) glDeleteBuffers(1, &bobVBO);
    if (bobEBO != 0) glDeleteBuffers(1, &bobEBO);
}

void Pendulum::GenerateRodVertices() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float rodWidth = 0.01f;
    float w = rodWidth / 2.0f;
    float l = pendulumLength;
    
    vertices.insert(vertices.end(), {
        -w,  0.0f,  w,    0.0f, 0.0f, 1.0f,    0.3f, 0.2f, 0.1f, 
         w,  0.0f,  w,    0.0f, 0.0f, 1.0f,    0.3f, 0.2f, 0.1f,
         w,   -l,  w,    0.0f, 0.0f, 1.0f,    0.3f, 0.2f, 0.1f,
        -w,   -l,  w,    0.0f, 0.0f, 1.0f,    0.3f, 0.2f, 0.1f,
        
        -w,  0.0f, -w,    0.0f, 0.0f, -1.0f,   0.3f, 0.2f, 0.1f,
         w,  0.0f, -w,    0.0f, 0.0f, -1.0f,   0.3f, 0.2f, 0.1f,
         w,   -l, -w,    0.0f, 0.0f, -1.0f,   0.3f, 0.2f, 0.1f,
        -w,   -l, -w,    0.0f, 0.0f, -1.0f,   0.3f, 0.2f, 0.1f,
        
        -w,  0.0f,  w,   -1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
        -w,  0.0f, -w,   -1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
        -w,   -l, -w,   -1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
        -w,   -l,  w,   -1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
        
         w,  0.0f,  w,    1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
         w,  0.0f, -w,    1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
         w,   -l, -w,    1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
         w,   -l,  w,    1.0f, 0.0f, 0.0f,    0.25f, 0.15f, 0.1f,
        
        -w,  0.0f,  w,    0.0f, 1.0f, 0.0f,    0.35f, 0.25f, 0.15f,
         w,  0.0f,  w,    0.0f, 1.0f, 0.0f,    0.35f, 0.25f, 0.15f,
         w,  0.0f, -w,    0.0f, 1.0f, 0.0f,    0.35f, 0.25f, 0.15f,
        -w,  0.0f, -w,    0.0f, 1.0f, 0.0f,    0.35f, 0.25f, 0.15f,
        
        -w,   -l,  w,    0.0f, -1.0f, 0.0f,   0.2f, 0.1f, 0.05f,
         w,   -l,  w,    0.0f, -1.0f, 0.0f,   0.2f, 0.1f, 0.05f,
         w,   -l, -w,    0.0f, -1.0f, 0.0f,   0.2f, 0.1f, 0.05f,
        -w,   -l, -w,    0.0f, -1.0f, 0.0f,   0.2f, 0.1f, 0.05f,
    });
    
    indices.insert(indices.end(), {
        0, 1, 2,  2, 3, 0,
        4, 6, 5,  6, 4, 7,
        8, 9, 10,  10, 11, 8,
        12, 13, 14,  14, 15, 12,
        16, 17, 18,  18, 19, 16,
        20, 21, 22,  22, 23, 20
    });
    
    rodIndexCount = static_cast<unsigned int>(indices.size());
    
    glGenVertexArrays(1, &rodVAO);
    glGenBuffers(1, &rodVBO);
    glGenBuffers(1, &rodEBO);
    
    glBindVertexArray(rodVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rodVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rodEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Pendulum::GenerateBobVertices() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    int segments = 16;
    float r = bobRadius;

    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(1.0f);
    vertices.push_back(0.2f); vertices.push_back(0.1f); vertices.push_back(0.05f);  
    
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159265359f * i / segments;
        float x = r * cos(angle);
        float y = r * sin(angle);
        
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(r);
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(1.0f);
        vertices.push_back(0.2f); vertices.push_back(0.1f); vertices.push_back(0.05f);
        
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(-r);
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-1.0f);
        vertices.push_back(0.2f); vertices.push_back(0.1f); vertices.push_back(0.05f);
    }
    
    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(1 + i * 2);
        indices.push_back(1 + (i + 1) * 2);
    }
    
    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(1 + (i + 1) * 2 + 1);
        indices.push_back(1 + i * 2 + 1);
    }
    
    for (int i = 0; i < segments; i++) {
        int curr = 1 + i * 2;
        int next = 1 + (i + 1) * 2;
        int currBack = curr + 1;
        int nextBack = next + 1;
        
        indices.push_back(curr);
        indices.push_back(next);
        indices.push_back(nextBack);
        indices.push_back(nextBack);
        indices.push_back(currBack);
        indices.push_back(curr);
    }
    
    bobIndexCount = static_cast<unsigned int>(indices.size());
    
    glGenVertexArrays(1, &bobVAO);
    glGenBuffers(1, &bobVBO);
    glGenBuffers(1, &bobEBO);
    
    glBindVertexArray(bobVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bobVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bobEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Pendulum::SetAmplitude(float amp) {
    amplitude = amp;
}

void Pendulum::SetPeriod(float per) {
    period = per;
}

void Pendulum::Start() {
    isMoving = true;
}

void Pendulum::Stop() {
    isMoving = false;
}

void Pendulum::Update(float deltaTime) {
    if (isMoving) {
        currentTime += deltaTime;
        UpdatePendulumPosition();
    }
}

void Pendulum::UpdatePendulumPosition() {
    float angle = amplitude * sin(2.0f * 3.14159265359f * currentTime / period);
    
    SetRotation(glm::vec3(0.0f, 0.0f, angle));
}

void Pendulum::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();
    
    glm::mat4 model = GetModelMatrix();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    shader.setVec3("lightPos", glm::vec3(3.0f, 3.0f, 3.0f));
    shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    
    glBindVertexArray(rodVAO);
    glDrawElements(GL_TRIANGLES, rodIndexCount, GL_UNSIGNED_INT, 0);
    
    glm::mat4 bobModel = model;
    bobModel = glm::translate(bobModel, glm::vec3(0.0f, -pendulumLength, 0.0f));
    shader.setMat4("model", bobModel);
    
    glBindVertexArray(bobVAO);
    glDrawElements(GL_TRIANGLES, bobIndexCount, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
}

