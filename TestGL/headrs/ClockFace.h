#pragma once
#ifndef CLOCK_FACE_H
#define CLOCK_FACE_H

#include "ClockComponent.h"

class ClockFace : public ClockComponent {
public:
    ClockFace(float radius = 0.45f, int segments = 32);
    virtual ~ClockFace();

    virtual void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) override;

private:
    float faceRadius;
    int faceSegments;
    
    void GenerateFaceVertices();
};

#endif
