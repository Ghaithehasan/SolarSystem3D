#include "NumberMarker.h"
#include <vector>
#include <cmath>

NumberMarker::NumberMarker(int number)
    : markerNumber(number), markerSize(0.06f)  
{
    if (number < 1 || number > 12) {
        markerNumber = 1;  
    }
    GenerateMarkerVertices();
}

NumberMarker::~NumberMarker() {
}

void NumberMarker::GenerateMarkerVertices() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float s = markerSize / 2.0f;
    float thickness = 0.025f; 

    float r = 0.15f, g = 0.15f, b = 0.15f;

    vertices.insert(vertices.end(), {
        -s, -s,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,
         s, -s,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,
         s,  s,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,
        -s,  s,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,

        // Back face
        -s, -s, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,
         s, -s, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,
         s,  s, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,
        -s,  s, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,

        // Left side
        -s, -s,  thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,
        -s, -s, -thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,
        -s,  s, -thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,
        -s,  s,  thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,

        // Right side
         s, -s,  thickness,    1.0f, 0.0f, 0.0f,    r, g, b,
         s, -s, -thickness,    1.0f, 0.0f, 0.0f,    r, g, b,
         s,  s, -thickness,    1.0f, 0.0f, 0.0f,    r, g, b,
         s,  s,  thickness,    1.0f, 0.0f, 0.0f,    r, g, b,

         // Top
         -s,  s,  thickness,    0.0f, 1.0f, 0.0f,    r, g, b,
          s,  s,  thickness,    0.0f, 1.0f, 0.0f,    r, g, b,
          s,  s, -thickness,    0.0f, 1.0f, 0.0f,    r, g, b,
         -s,  s, -thickness,    0.0f, 1.0f, 0.0f,    r, g, b,

         // Bottom
         -s, -s,  thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
          s, -s,  thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
          s, -s, -thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
         -s, -s, -thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
        });

    indices.insert(indices.end(), {
        // Front
        0, 1, 2,  2, 3, 0,
        // Back
        4, 6, 5,  6, 4, 7,
        // Left
        8, 9, 10,  10, 11, 8,
        // Right
        12, 13, 14,  14, 15, 12,
        // Top
        16, 17, 18,  18, 19, 16,
        // Bottom
        20, 21, 22,  22, 23, 20
        });

    SetupBuffers(vertices.data(), vertices.size() * sizeof(float),
        indices.data(), indices.size());
}

void NumberMarker::SetPositionOnFace(float faceRadius) {
    CalculatePosition(faceRadius);
}

void NumberMarker::CalculatePosition(float faceRadius) {
    float angle = (markerNumber * 30.0f - 90.0f) * 3.14159265359f / 180.0f;

    float distance = faceRadius * 0.80f; 
    float x = distance * cos(angle);
    float y = distance * sin(angle);

    SetPosition(glm::vec3(x, y, 0.025f));
}

void NumberMarker::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();

    glm::mat4 model = GetModelMatrix();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    shader.setVec3("lightPos", glm::vec3(3.0f, 3.0f, 3.0f));
    shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}