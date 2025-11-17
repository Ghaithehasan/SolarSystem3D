#pragma once
#ifndef CLOCK_COMPONENT_H
#define CLOCK_COMPONENT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class ClockComponent {
public:
    ClockComponent();
    virtual ~ClockComponent();

    virtual void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) = 0;

    virtual void Update(float deltaTime) {}

    void SetPosition(const glm::vec3& position);
    glm::vec3 GetPosition() const { return position; }

    void SetRotation(const glm::vec3& rotation);
    glm::vec3 GetRotation() const { return rotation; }

    void SetScale(const glm::vec3& scale);
    glm::vec3 GetScale() const { return scale; }

protected:
    glm::vec3 position;
    glm::vec3 rotation;     
    glm::vec3 scale;

    GLuint VAO, VBO, EBO;
    unsigned int indexCount;

    glm::mat4 GetModelMatrix() const;

    void SetupBuffers(const float* vertices, size_t vertexSize, 
                     const unsigned int* indices, size_t indexCount);
};

#endif
