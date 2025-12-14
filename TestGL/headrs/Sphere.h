#pragma once
#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

class Sphere {
public:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    Sphere(float radius = 1.0f, int sectors = 36, int stacks = 18);
    ~Sphere();
    void Draw();

private:
    void generateSphere(float radius, int sectors, int stacks);
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

#endif

