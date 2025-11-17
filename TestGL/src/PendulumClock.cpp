#include "PendulumClock.h"

PendulumClock::PendulumClock()
    : clockSize(1.0f), faceRadius(0.45f)
{
    InitializeComponents();
}

PendulumClock::~PendulumClock() {
}

void PendulumClock::InitializeComponents() {
    box = std::make_unique<ClockBox>(1.0f, 1.0f, 0.5f);
    box->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    face = std::make_unique<ClockFace>(faceRadius, 32);
    face->SetPosition(glm::vec3(0.0f, 0.0f, 0.15f));  
    
    hourHand = std::make_unique<ClockHand>(ClockHand::HOUR_HAND);
    hourHand->SetPosition(glm::vec3(0.0f, 0.0f, 0.2f));  
    
    minuteHand = std::make_unique<ClockHand>(ClockHand::MINUTE_HAND);
    minuteHand->SetPosition(glm::vec3(0.0f, 0.0f, 0.21f));  
    
    hourHand->SetTime(10, 30);
    minuteHand->SetTime(10, 30);

    pendulum = std::make_unique<Pendulum>(0.8f, 0.05f);
    pendulum->SetPosition(glm::vec3(0.0f, -0.5f, 0.0f));
    pendulum->SetAmplitude(0.5f);  
    pendulum->SetPeriod(2.0f);     
    pendulum->Start();
    
    for (int i = 1; i <= 12; i++) {
        auto marker = std::make_unique<NumberMarker>(i);
        marker->SetPositionOnFace(faceRadius);
        marker->SetPosition(glm::vec3(
            marker->GetPosition().x,
            marker->GetPosition().y,
            0.16f  
        ));
        numberMarkers.push_back(std::move(marker));
    }
}

void PendulumClock::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos) {
    shader.use();
    shader.setVec3("viewPos", viewPos);
    
    box->Draw(shader, view, projection);
    
    face->Draw(shader, view, projection);
    
    for (auto& marker : numberMarkers) {
        marker->Draw(shader, view, projection);
    }
    
    shader.use();
    shader.setVec3("viewPos", viewPos);
    hourHand->Draw(shader, view, projection);
    
    shader.use();
    shader.setVec3("viewPos", viewPos);
    minuteHand->Draw(shader, view, projection);
    
    pendulum->Draw(shader, view, projection);
}

void PendulumClock::Update(float deltaTime) {
    pendulum->Update(deltaTime);
    
}

void PendulumClock::SetTime(int hours, int minutes) {
    hourHand->SetTime(hours, minutes);
    minuteHand->SetTime(hours, minutes);
}

void PendulumClock::StartPendulum() {
    pendulum->Start();
}

void PendulumClock::StopPendulum() {
    pendulum->Stop();
}

