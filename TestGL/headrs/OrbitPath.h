#pragma once
#ifndef ORBIT_PATH_H
#define ORBIT_PATH_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

class OrbitPath {
public:
    unsigned int VAO, VBO;
    unsigned int pointCount;

    OrbitPath();
    ~OrbitPath();
    void generateEarthOrbit(float semiMajor, float semiMinor, int segments = 100);
    void generateMoonOrbit(float radius, int segments = 100);
    void Draw();

private:
    std::vector<float> vertices;
};

#endif

