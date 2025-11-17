#include "ClockFace.h"
#include <vector>
#include <cmath>

ClockFace::ClockFace(float radius, int segments)
    : faceRadius(radius), faceSegments(segments)
{
    GenerateFaceVertices();
}

ClockFace::~ClockFace() {
}

void ClockFace::GenerateFaceVertices() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.01f);  // Position
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(1.0f);     // Normal
    vertices.push_back(0.95f); vertices.push_back(0.95f); vertices.push_back(0.9f);  // Color (أبيض فاتح)
    
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-0.01f);  // Position
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-1.0f);   // Normal
    vertices.push_back(0.95f); vertices.push_back(0.95f); vertices.push_back(0.9f); // Color
    
    for (int i = 0; i <= faceSegments; i++) {
        float angle = 2.0f * 3.14159265359f * i / faceSegments;
        float x = faceRadius * cos(angle);
        float y = faceRadius * sin(angle);
        
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(0.01f);  // Position
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(1.0f);  // Normal
        vertices.push_back(0.95f); vertices.push_back(0.95f); vertices.push_back(0.9f); // Color
        
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(-0.01f); // Position
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-1.0f); // Normal
        vertices.push_back(0.95f); vertices.push_back(0.95f); vertices.push_back(0.9f); // Color
    }
    
    for (int i = 0; i < faceSegments; i++) {
        indices.push_back(0);  // Center
        indices.push_back(2 + i * 2);  // Current vertex
        indices.push_back(2 + (i + 1) * 2);  // Next vertex
    }
    
    for (int i = 0; i < faceSegments; i++) {
        indices.push_back(1);  // Center
        indices.push_back(2 + (i + 1) * 2 + 1);  // Next vertex (back)
        indices.push_back(2 + i * 2 + 1);  // Current vertex (back)
    }
    
    SetupBuffers(vertices.data(), vertices.size() * sizeof(float),
                indices.data(), indices.size());
}

void ClockFace::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();
    
    glm::mat4 model = GetModelMatrix();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    shader.setVec3("lightPos", glm::vec3(3.0f, 3.0f, 3.0f));
    shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

