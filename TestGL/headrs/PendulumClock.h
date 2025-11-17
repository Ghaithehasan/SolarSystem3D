#pragma once
#ifndef PENDULUM_CLOCK_H
#define PENDULUM_CLOCK_H

#include "ClockBox.h"
#include "ClockFace.h"
#include "ClockHand.h"
#include "NumberMarker.h"
#include "Pendulum.h"
#include <vector>
#include <memory>

class PendulumClock {
public:
    PendulumClock();
    ~PendulumClock();

    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);

    void Update(float deltaTime);

    void SetTime(int hours, int minutes);

    void StartPendulum();
    void StopPendulum();

private:
    std::unique_ptr<ClockBox> box;
    std::unique_ptr<ClockFace> face;
    std::unique_ptr<ClockHand> hourHand;
    std::unique_ptr<ClockHand> minuteHand;
    std::unique_ptr<Pendulum> pendulum;
    
    std::vector<std::unique_ptr<NumberMarker>> numberMarkers;
    
    float clockSize;
    float faceRadius;
    
    void InitializeComponents();
};

#endif
