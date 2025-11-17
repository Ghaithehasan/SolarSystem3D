#include "ClockHand.h"
#include <vector>
#include <cmath>

ClockHand::ClockHand(HandType type)
    : handType(type), currentAngle(0.0f)
{
    if (type == HOUR_HAND) {
        handLength = 0.35f;  
        handWidth = 0.035f; 
    }
    else if (type == MINUTE_HAND) {
        handLength = 0.45f;  
        handWidth = 0.025f; 
    }
    else {  
        handLength = 0.40f;  
        handWidth = 0.015f; 
    }

    GenerateHandVertices();
    std::cout << "ClockHand created: type=" << (type == HOUR_HAND ? "HOUR" : (type == MINUTE_HAND ? "MINUTE" : "SECOND"))
        << ", length=" << handLength << ", width=" << handWidth
        << ", VAO=" << VAO << ", indexCount=" << indexCount << std::endl;
}

ClockHand::~ClockHand() {
}

void ClockHand::GenerateHandVertices() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float w = handWidth / 2.0f;
    float l = handLength;
    float thickness = 0.03f;  


    float r, g, b;
    if (handType == HOUR_HAND) {
        r = 0.3f; g = 0.3f; b = 0.3f;  
    }
    else if (handType == MINUTE_HAND) {
        r = 0.25f; g = 0.25f; b = 0.25f;  
    }
    else {  
        r = 0.8f; g = 0.1f; b = 0.1f;  
    }

  
    vertices.insert(vertices.end(), {
        -w,  0.0f,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,
         w,  0.0f,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,
         w,    l,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,
        -w,    l,  thickness,    0.0f, 0.0f, 1.0f,    r, g, b,

        -w,  0.0f, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,
         w,  0.0f, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,
         w,    l, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,
        -w,    l, -thickness,    0.0f, 0.0f, -1.0f,   r, g, b,

        -w,  0.0f,  thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,
        -w,  0.0f, -thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,
        -w,    l, -thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,
        -w,    l,  thickness,   -1.0f, 0.0f, 0.0f,    r, g, b,

         w,  0.0f,  thickness,    1.0f, 0.0f, 0.0f,    r, g, b,
         w,  0.0f, -thickness,    1.0f, 0.0f, 0.0f,    r, g, b,
         w,    l, -thickness,    1.0f, 0.0f, 0.0f,    r, g, b,
         w,    l,  thickness,    1.0f, 0.0f, 0.0f,    r, g, b,

         -w,    l,  thickness,    0.0f, 1.0f, 0.0f,    r, g, b,
          w,    l,  thickness,    0.0f, 1.0f, 0.0f,    r, g, b,
          w,    l, -thickness,    0.0f, 1.0f, 0.0f,    r, g, b,
         -w,    l, -thickness,    0.0f, 1.0f, 0.0f,    r, g, b,

         -w,  0.0f,  thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
          w,  0.0f,  thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
          w,  0.0f, -thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
         -w,  0.0f, -thickness,    0.0f, -1.0f, 0.0f,   r, g, b,
        });

    indices.insert(indices.end(), {
        0, 1, 2,  2, 3, 0,
        4, 6, 5,  6, 4, 7,
        8, 9, 10,  10, 11, 8,
        12, 13, 14,  14, 15, 12,
        16, 17, 18,  18, 19, 16,
        20, 21, 22,  22, 23, 20
        });

    SetupBuffers(vertices.data(), vertices.size() * sizeof(float),
        indices.data(), indices.size());
}

void ClockHand::SetTime(int hours, int minutes, int seconds) {
    CalculateAngle(hours, minutes, seconds);
}

void ClockHand::CalculateAngle(int hours, int minutes, int seconds) {
    if (handType == HOUR_HAND) {
        float hourAngle = (hours % 12) * 30.0f + minutes * 0.5f + seconds * (0.5f / 60.0f);
        currentAngle = glm::radians(hourAngle - 90.0f);  
    }
    else if (handType == MINUTE_HAND) {
        float minuteAngle = minutes * 6.0f + seconds * 0.1f;
        currentAngle = glm::radians(minuteAngle - 90.0f);  
    }
    else {  
        float secondAngle = seconds * 6.0f;
        currentAngle = glm::radians(secondAngle - 90.0f);  
    }

    SetRotation(glm::vec3(0.0f, 0.0f, currentAngle));
}

void ClockHand::Update(float deltaTime) {
}

void ClockHand::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    if (VAO == 0 || indexCount == 0) {
        std::cerr << "ERROR: ClockHand Draw - VAO=" << VAO << ", indexCount=" << indexCount << std::endl;
        return;
    }

    shader.use();

    glm::mat4 model = GetModelMatrix();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    shader.setVec3("lightPos", glm::vec3(3.0f, 3.0f, 3.0f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}