#pragma once
#include "render/model.h"
#include "render/input/inputserver.h"
#include "render/physics.h"
namespace Render
{
    struct ParticleEmitter;
}

namespace Game
{

struct Ball
{
    Ball();
    Physics::ColliderId collider;

    glm::vec3 position = glm::vec3(0,0.5f,0);
    float radius=0.175f;

    static glm::vec3 aimDirection;


   
    glm::vec3 scale = glm::vec3(5.0f, 5.0f, 5.0f);
    glm::mat4 transform = glm::mat4(1);
    glm::vec3 linearVelocity = glm::vec3(0);
    bool isStationary=true;
    bool fromLock = false;
    const float normalSpeed = 1.0f;

    const float accelerationFactor = 1.0f;
    float currentSpeed = 0.0f;

    float shotPower = 0.0f;
    const float maxShotPower = 20.0f;
    const float shotChargeSpeed = 3.0f;
    bool isChargingShot = false;
    
    const float cameraSmoothFactor = 5.0f;
    bool camLock = false;
    const float camOffsetY = 0.25f;
    glm::vec3 camPos = glm::vec3(0, 1.0f, -2.0f);

    float cancelHoldTime = 1.5f; // how long they must hold at max to cancel
    float maxPowerCount = 0.0f;
    bool canceledShot = false;
    
    Render::ModelId model;
    int shotTracker = 0;

    void Update(float dt,const Input::Gamepad& pad);

    Physics::RaycastPayload CheckCollisions();
    void SimulateCollision(glm::vec3 normal);
  
};

}