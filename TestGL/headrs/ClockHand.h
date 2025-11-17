#pragma once
#ifndef CLOCK_HAND_H
#define CLOCK_HAND_H

#include "ClockComponent.h"

class ClockHand : public ClockComponent {
public:
    enum HandType {
        HOUR_HAND,      
        MINUTE_HAND,    
        SECOND_HAND     
    };

    ClockHand(HandType type);
    virtual ~ClockHand();

    virtual void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) override;
    virtual void Update(float deltaTime) override;

    void SetTime(int hours, int minutes, int seconds = 0);

private:
    HandType handType;
    float handLength;
    float handWidth;
    float currentAngle;  
    
    void GenerateHandVertices();
    void CalculateAngle(int hours, int minutes, int seconds = 0);
};

#endif
