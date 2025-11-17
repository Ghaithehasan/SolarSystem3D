#pragma once
#ifndef CLOCK_BOX_H
#define CLOCK_BOX_H

#include "ClockComponent.h"

class ClockBox : public ClockComponent {
public:
    ClockBox(float width = 1.0f, float height = 1.0f, float depth = 0.3f);
    virtual ~ClockBox();

    virtual void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) override;

private:
    float boxWidth;
    float boxHeight;
    float boxDepth;
    
    void GenerateBoxVertices();
};

#endif
