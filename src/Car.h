#ifndef CAR_H
#define CAR_H

#include "Physics.h"

class Car {
public:
    Car();
    ~Car();
    
    void Init();
    void Update(float dt, class Track& track);
    void Render(float offsetX);
    
    void Accelerate(float dt);
    void Brake(float dt);
    void Handbrake(float dt);
    void TurnLeft(float dt);
    void TurnRight(float dt);
    void Reset();
    void SetSteeringInput(float input);
    
    float GetSpeed() const { return speed; }
    float GetMaxSpeed() const { return maxSpeed; }
    float GetProgress() const { return progress; }
    float GetRotation() const { return steeringAngle; }
    
private:
    float speed;
    float progress;
    float steeringAngle;
    float steeringInput;
    
    float maxSpeed;
    float acceleration;
    float brakingForce;
    float friction;
    float turnSpeed;
    
    float width;
    float length;
};

#endif
