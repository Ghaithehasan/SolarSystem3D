#include "ClockComponent.h"

ClockComponent::ClockComponent()
    : position(0.0f, 0.0f, 0.0f),
      rotation(0.0f, 0.0f, 0.0f),
      scale(1.0f, 1.0f, 1.0f),
      VAO(0), VBO(0), EBO(0),
      indexCount(0)
{
}

ClockComponent::~ClockComponent() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
    }
}

void ClockComponent::SetPosition(const glm::vec3& pos) {
    position = pos;
}

void ClockComponent::SetRotation(const glm::vec3& rot) {
    rotation = rot;
}

void ClockComponent::SetScale(const glm::vec3& scl) {
    scale = scl;
}

glm::mat4 ClockComponent::GetModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    
    model = glm::translate(model, position);
    
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    
    model = glm::scale(model, scale);
    
    return model;
}

void ClockComponent::SetupBuffers(const float* vertices, size_t vertexSize,
                                 const unsigned int* indices, size_t indexCount) {
    this->indexCount = static_cast<unsigned int>(indexCount);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

