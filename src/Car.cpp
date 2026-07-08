#include "Car.h"
#include "Track.h"
#include <cmath>

Car::Car() : speed(0.0f), progress(0.0f), steeringAngle(0.0f), steeringInput(0.0f),
              maxSpeed(3.5f), acceleration(5.0f), brakingForce(10.0f),
              friction(2.0f), turnSpeed(2.8f),
              width(2.0f), length(4.0f) {
}

Car::~Car() {
}

void Car::Init() {
    speed = 0.0f;
    progress = 0.0f;
    steeringAngle = 0.0f;
    steeringInput = 0.0f;
}

void Car::Update(float dt, Track& track) {
    if (speed > 0.0f) {
        speed -= friction * dt;
        if (speed < 0.0f) speed = 0.0f;
    }

    progress += speed * dt;
    if (progress > track.GetTrackLength()) {
        progress -= track.GetTrackLength();
    }

    const float maxSteer = 1.0f;
    if (steeringInput < -0.01f) {
        steeringAngle -= turnSpeed * dt;
    } else if (steeringInput > 0.01f) {
        steeringAngle += turnSpeed * dt;
    } else {
        float returnRate = turnSpeed * 2.5f * dt;
        if (steeringAngle > 0.0f) {
            steeringAngle -= returnRate;
            if (steeringAngle < 0.0f) steeringAngle = 0.0f;
        } else if (steeringAngle < 0.0f) {
            steeringAngle += returnRate;
            if (steeringAngle > 0.0f) steeringAngle = 0.0f;
        }
    }

    if (steeringAngle > maxSteer) steeringAngle = maxSteer;
    if (steeringAngle < -maxSteer) steeringAngle = -maxSteer;

    steeringInput = 0.0f;
}

void Car::SetSteeringInput(float input) {
    if (input < -1.0f) input = -1.0f;
    if (input > 1.0f) input = 1.0f;
    steeringInput = input;
}

void Car::Accelerate(float dt) {
    speed += acceleration * dt;
    if (speed > maxSpeed) speed = maxSpeed;
}

void Car::Brake(float dt) {
    speed -= brakingForce * dt;
    if (speed < -maxSpeed * 0.3f) speed = -maxSpeed * 0.3f;
}

void Car::Handbrake(float dt) {
    (void)dt;
    speed *= 0.9f;
}

void Car::TurnLeft(float dt) {
    (void)dt;
    SetSteeringInput(-1.0f);
}

void Car::TurnRight(float dt) {
    (void)dt;
    SetSteeringInput(1.0f);
}

void Car::Reset() {
    speed = 0.0f;
    progress = 0.0f;
    steeringAngle = 0.0f;
    steeringInput = 0.0f;
}

void Car::Render(float offsetX) {
    (void)offsetX;
}
