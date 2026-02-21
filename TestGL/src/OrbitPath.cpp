// =============================================================
// SolarSystem3D - OrbitPath.cpp
// Generates and renders elliptical/circular orbit path lines
// using OpenGL VAO/VBO. Orbits are drawn as GL_LINE_STRIP.
// =============================================================

#include "OrbitPath.h"
#include <cmath>
#include <iostream>

// Constructor: allocate GPU buffers (VAO and VBO) for orbit geometry
OrbitPath::OrbitPath() {
    glGenVertexArrays(1, &VAO); // Vertex Array Object stores vertex attribute config
    glGenBuffers(1, &VBO);      // Vertex Buffer Object stores actual vertex data
    pointCount = 0;
}

// Destructor: release GPU memory when the object goes out of scope
OrbitPath::~OrbitPath() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Generates an elliptical orbit path for Earth using parametric equations:
// x = semiMajor * cos(angle),  z = semiMinor * sin(angle)
// semiMajor: half-width of the ellipse (X axis)
// semiMinor: half-depth of the ellipse (Z axis)
// segments: number of line segments (higher = smoother ellipse)
void OrbitPath::generateEarthOrbit(float semiMajor, float semiMinor, int segments) {
    vertices.clear();
    const float PI = 3.14159265359f;
    
    // Sample evenly-spaced points around the full ellipse
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments; // Angle in radians [0, 2pi]
        float x = semiMajor * cosf(angle);       // Horizontal (width) component
        float z = semiMinor * sinf(angle);        // Depth component
        
        // All orbit points lie on Y=0 (the orbital plane)
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }
    
    // Each vertex has 3 floats (x, y, z), so divide total by 3 for point count
    pointCount = vertices.size() / 3;
    
    // Upload vertex data to the GPU
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    // Attribute 0: position (vec3), tightly packed
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind to prevent accidental modification
}

// Generates a circular orbit path for the Moon using parametric equations:
// x = radius * cos(angle),  z = radius * sin(angle)
// radius: orbit radius (circle, not ellipse)
// segments: number of line segments
void OrbitPath::generateMoonOrbit(float radius, int segments) {
    vertices.clear();
    const float PI = 3.14159265359f;
    
    // Sample evenly-spaced points around the full circle
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        
        // Keep on the orbital plane (Y = 0)
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }
    
    pointCount = vertices.size() / 3;
    
    // Upload to GPU (same layout as Earth orbit)
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// Renders the orbit path as a connected line strip
void OrbitPath::Draw() {
    glBindVertexArray(VAO);
    // GL_LINE_STRIP connects each vertex to the next with a line segment
    glDrawArrays(GL_LINE_STRIP, 0, pointCount);
    glBindVertexArray(0);
}