#include "PendulumClock.h"

PendulumClock::PendulumClock()
    : clockSize(1.0f), faceRadius(0.45f),
      currentHours(10), currentMinutes(30), currentSeconds(0),
      initialPendulumTime(0.0f), initialTotalSeconds(0)
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
    
    secondHand = std::make_unique<ClockHand>(ClockHand::SECOND_HAND);
    secondHand->SetPosition(glm::vec3(0.0f, 0.0f, 0.22f)); 
    
    hourHand->SetTime(currentHours, currentMinutes, currentSeconds);
    minuteHand->SetTime(currentHours, currentMinutes, currentSeconds);
    secondHand->SetTime(currentHours, currentMinutes, currentSeconds);

    pendulum = std::make_unique<Pendulum>(0.8f, 0.05f);
    pendulum->SetPosition(glm::vec3(0.0f, -0.5f, 0.0f));
    pendulum->SetAmplitude(0.5f);  
    pendulum->SetPeriod(2.0f); 
    pendulum->Start();
    
    initialPendulumTime = pendulum->GetCurrentTime();
    initialTotalSeconds = currentHours * 3600 + currentMinutes * 60 + currentSeconds;
    
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
    
    shader.use();
    shader.setVec3("viewPos", viewPos);
    secondHand->Draw(shader, view, projection);
    
    pendulum->Draw(shader, view, projection);
}

void PendulumClock::Update(float deltaTime) {
    pendulum->Update(deltaTime);
    
    float pendulumTime = pendulum->GetCurrentTime();
    float pendulumPeriod = pendulum->GetPeriod(); 
    
    float elapsedPendulumTime = pendulumTime - initialPendulumTime;
    
    float cyclesCompleted = elapsedPendulumTime / pendulumPeriod;
    
    int totalSecondsElapsed = static_cast<int>(cyclesCompleted * 2.0f);
    
    int newTotalSeconds = initialTotalSeconds + totalSecondsElapsed;
    
    int newSeconds = newTotalSeconds % 60;
    int newMinutes = (newTotalSeconds / 60) % 60;
    int newHours = (newTotalSeconds / 3600) % 12;
    
    if (newSeconds != currentSeconds || newMinutes != currentMinutes || newHours != currentHours) {
        currentSeconds = newSeconds;
        currentMinutes = newMinutes;
        currentHours = newHours;
        
        hourHand->SetTime(currentHours, currentMinutes, currentSeconds);
        minuteHand->SetTime(currentHours, currentMinutes, currentSeconds);
        secondHand->SetTime(currentHours, currentMinutes, currentSeconds);
    }
}

void PendulumClock::SetTime(int hours, int minutes, int seconds) {
    currentHours = hours % 12;
    currentMinutes = minutes % 60;
    currentSeconds = seconds % 60;
    
    initialPendulumTime = pendulum->GetCurrentTime();
    initialTotalSeconds = currentHours * 3600 + currentMinutes * 60 + currentSeconds;
    
    hourHand->SetTime(currentHours, currentMinutes, currentSeconds);
    minuteHand->SetTime(currentHours, currentMinutes, currentSeconds);
    secondHand->SetTime(currentHours, currentMinutes, currentSeconds);
}

void PendulumClock::StartPendulum() {
    pendulum->Start();
}

void PendulumClock::StopPendulum() {
    pendulum->Stop();
}

