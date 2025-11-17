#pragma once
#ifndef NUMBER_MARKER_H
#define NUMBER_MARKER_H

#include "ClockComponent.h"

class NumberMarker : public ClockComponent {
public:
    NumberMarker(int number); 
    virtual ~NumberMarker();

    virtual void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) override;

    void SetPositionOnFace(float faceRadius);

private:
    int markerNumber;
    float markerSize;
    
    void GenerateMarkerVertices();
    void CalculatePosition(float faceRadius);
};

#endif
