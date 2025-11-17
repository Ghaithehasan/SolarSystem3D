#pragma once
#ifndef PENDULUM_H
#define PENDULUM_H

#include "ClockComponent.h"

class Pendulum : public ClockComponent {
public:
    Pendulum(float length = 0.8f, float bobRadius = 0.05f);
    virtual ~Pendulum();

    virtual void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) override;
    virtual void Update(float deltaTime) override;

    void SetAmplitude(float amplitude);  
    void SetPeriod(float period);        
    void Start();                        
    void Stop();
    
    float GetCurrentTime() const { return currentTime; }
    float GetPeriod() const { return period; }                         

private:
    float pendulumLength;
    float bobRadius;
    float amplitude;      
    float period;         
    float currentTime;    
    bool isMoving;        
    
    GLuint rodVAO, rodVBO, rodEBO;
    GLuint bobVAO, bobVBO, bobEBO;
    unsigned int rodIndexCount;
    unsigned int bobIndexCount;
    
    void GenerateRodVertices();
    void GenerateBobVertices();
    void UpdatePendulumPosition();
};

#endif
