#include "OrbitPath.h"
#include <cmath>
#include <iostream>

OrbitPath::OrbitPath() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    pointCount = 0;
}

OrbitPath::~OrbitPath() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void OrbitPath::generateEarthOrbit(float semiMajor, float semiMinor, int segments) {
    vertices.clear();
    const float PI = 3.14159265359f;
    
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float x = semiMajor * cosf(angle);
        float z = semiMinor * sinf(angle);
        
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }
    
    pointCount = vertices.size() / 3;
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void OrbitPath::generateMoonOrbit(float radius, int segments) {
    vertices.clear();
    const float PI = 3.14159265359f;
    
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }
    
    pointCount = vertices.size() / 3;
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void OrbitPath::Draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, pointCount);
    glBindVertexArray(0);
}

