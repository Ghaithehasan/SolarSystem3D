#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>

class Skybox {
public:
    unsigned int VAO, VBO;
    unsigned int textureID;

    Skybox();
    ~Skybox();
    void Draw();
    void loadTexture(const char* path);

private:
    void setupSkybox();
};

#endif

