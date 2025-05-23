#include "config.h"
#include "Ball.h"

#include "render/cameramanager.h"
#include "render/physics.h"
#include "render/debugrender.h"
#include "render/particlesystem.h"


using namespace Input;
using namespace glm;
using namespace Render;

namespace Game
{
glm::vec3 Ball::aimDirection = glm::vec3(0, 0, 1);
Ball::Ball()
{

    transform = glm::scale(scale);
}

void
Ball::Update(float dt,const Input::Gamepad& pad)
{
    Mouse* mouse = Input::GetDefaultMouse();
    Keyboard* kbd = Input::GetDefaultKeyboard();
   
   
    Camera* cam = CameraManager::GetCamera(CAMERA_MAIN);
   
    if (kbd->held[Key::W])
    {
        
         this->currentSpeed = mix(this->currentSpeed, this->normalSpeed, std::min(1.0f, dt * 90.0f));
    }
    if (kbd->pressed[Key::S])
    {

        this->currentSpeed = 0;
    }
    else
    {
        //this->currentSpeed = 0;
    }  
    static bool yWasPressed = false;

    if (pad.pressed[Input::Buttons::Y])
    {
        if (!yWasPressed)
        {
            camLock = !camLock;
            yWasPressed = true;
        }
    }
    else
    {
        yWasPressed = false;
    }
  
   
    vec3 desiredVelocity = aimDirection * currentSpeed;

    if (isStationary == false && !isChargingShot)
    {
        if (currentSpeed > 0.1f)
        {
            vec3 desiredVelocity = aimDirection * currentSpeed;
            this->linearVelocity = mix(this->linearVelocity, desiredVelocity, dt * accelerationFactor);
        }
        else // If just rolling after shot, apply friction
        {
            float frictionCoefficient = 0.65f; // tweak this for faster/slower roll decay
            this->linearVelocity *= 1.0f / (1.0f + dt * frictionCoefficient);

            if (glm::length(this->linearVelocity) < 0.05f)
            {
                this->linearVelocity = glm::vec3(0.0f);
                isStationary = true;
            }
        }
    }
    

    
    Physics::RaycastPayload raycast = CheckCollisions();
    if (raycast.hit)
    {
       // Debug::DrawLine(raycast.hitPoint, raycast.hitPoint + raycast.hitNormal * 1.0f, 1.0f, { 1, 1, 0, 1 }, { 1, 1, 0, 1 });
    }
    if (raycast.hit && raycast.hitDistance <= glm::length(linearVelocity) * dt + radius)
    {
        // Move to the hit point (with slight buffer to avoid jitter)
        float safeDistance = glm::max(raycast.hitDistance - radius * 0.95f, 0.0f);
        glm::vec3 travelDir = glm::normalize(linearVelocity);
        this->position += travelDir * safeDistance;
        
        // Simulate bounce
        SimulateCollision(raycast.hitNormal);

        // Update transform after bouncing
        this->transform[3] = glm::vec4(this->position, 1.0f);
    }
    else
    {
        // No hit: full movement
        this->position += this->linearVelocity * dt;
        this->transform[3] = glm::vec4(this->position, 1.0f);
    }

    
    Physics::SetTransform(collider, transform);
    
   
    isStationary = glm::length(linearVelocity) < 0.01f;

    
    // update camera view transform
    if (camLock)
    {
        glm::vec2 stickInput(pad.leftStick.x, pad.leftStick.y);
        float aimRotateSpeed = 2.0f; // tweak for sensitivity
        if (!pad.pressed[Input::Buttons::A] && isStationary)
        {
            
            if (fabs(stickInput.x) > 0.02f) // Deadzone on X axis
            {
                float angleDelta = -stickInput.x * aimRotateSpeed * dt;

                // Rotate aimDirection around Y axis
                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angleDelta, glm::vec3(0, 1, 0));
                aimDirection = glm::normalize(glm::vec3(rotation * glm::vec4(aimDirection, 0.0f)));
            }
        }
        if (isStationary)
        {
            Debug::DrawLine(position, position + aimDirection * 2.0f, 2.0f, glm::vec4(0, 0, 1, 1), glm::vec4(0, 0, 1, 1));
        }
        
     
        float camHeight = 5.0f;
       

        glm::vec2 rightStick(pad.rightStick.x, pad.rightStick.y);
        float deadzone = 0.1f;
        if (rightStick.y < 0.0f)
        {
            rightStick.y = 0.0f;
        }
           
        if (glm::length(rightStick) < deadzone)
        {
            rightStick = glm::vec2(0.0f); 
        }
           
        
      
         // tweak this for how far away from the ball you want to look
        glm::vec3 localforward = glm::normalize(glm::vec3(aimDirection.x, 0.0f, aimDirection.z));
        glm::vec3 localright = glm::normalize(glm::cross(localforward, glm::vec3(0, 1, 0)));
        float lookOffsetDist = 4.0f;
        glm::vec3 lookTargetOffset = (rightStick.x * localright + rightStick.y * localforward) * lookOffsetDist;
        glm::vec3 lookAtTarget = position + lookTargetOffset;

        // Camera stays directly above the ball
        glm::vec3 camPosition = position + glm::vec3(0.0f, camHeight, 0.0f);

        
        glm::vec3 camforward = glm::normalize(position - camPosition);
        glm::vec3 camright;
        if (!isStationary)
        {
           //camright = glm::normalize(glm::cross(camforward, linearVelocity));
            camright = glm::normalize(glm::cross(camforward, aimDirection));
        }
        else
        {
           camright = glm::normalize(glm::cross(camforward, aimDirection));
        }
        
        glm::vec3 camupVector = glm::normalize(glm::cross(camright, camforward)); // Perpendicular to both, follows aimDirection
        
        camPos = camPosition;

        cam->view = glm::lookAt(camPosition, lookAtTarget, camupVector);
     
        if (pad.pressed[Input::Buttons::A]&&isStationary && !canceledShot)
        {
            isChargingShot = true;
            shotPower += dt * shotChargeSpeed;
            shotPower = glm::clamp(shotPower, 0.0f, maxShotPower + 0.1f);
            if (shotPower >= maxShotPower)
            {
                maxPowerCount += dt;
                if (maxPowerCount >= cancelHoldTime)
                {
                    canceledShot = true;
                }
            }

            // Optional: draw a debug power bar above the ball
            Debug::DrawLine(position + glm::vec3(0, 0.1f, 0), position + aimDirection * (shotPower * 0.15f), 2.0f,
                glm::vec4(1, 1, 0, 1), glm::vec4(1, 0, 0, 1), Debug::AlwaysOnTop);
        }
        else if (pad.released[Input::Buttons::A])
        {
            if (isChargingShot && !canceledShot)
            {
                // Shoot!
                linearVelocity = aimDirection * shotPower;
                camLock = false;
                shotTracker++;
            }

            // Reset states regardless
            isChargingShot = false;
            shotPower = 0.0f;
            maxPowerCount = 0.0f;
            canceledShot = false;
        }
        fromLock = true;
        
    }
    else if(!isStationary)
    {
        // Your usual follow camera code
        glm::vec3 offset = -glm::normalize(linearVelocity) * 1.0f + glm::vec3(0.0f, camOffsetY, 0.0f);
        glm::vec3 desiredCamPos = position + offset;
        camPos = mix(camPos, desiredCamPos, dt * cameraSmoothFactor);
        
        cam->view = glm::lookAt(camPos, position, glm::vec3(0, 1, 0));
        fromLock = false;
    }

    
       
    glm::vec3 ballPos = glm::vec3(transform[3]);
    glm::vec3 velocityEnd = ballPos + linearVelocity * scale; // scale just makes it more visible

    //Debug::DrawLine(ballPos, velocityEnd, 2.0f, glm::vec4(0, 0, 1, 1), glm::vec4(0, 0, 1, 1));
}

Physics::RaycastPayload
Ball::CheckCollisions()
{
 
    glm::vec3 forward = glm::normalize(linearVelocity);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))); // assuming Y-up
    glm::vec3 start = position;

    float rayLength = glm::length(linearVelocity) + radius;
    //float rayLength = 15.0f + radius;
    float sideOffset = radius * 0.9f;

    // Main ray
    Physics::RaycastPayload centerRay = Physics::Raycast(start, forward, rayLength, 2);
    //Debug::DrawLine(start, start + forward * rayLength, 1.0f, { 1, 0, 0, 1 }, { 1, 0, 0, 1 });

    // Side rays
    Physics::RaycastPayload leftRay = Physics::Raycast(start - right * sideOffset, forward, rayLength, 2);
    //Debug::DrawLine(start - right * sideOffset, (start - right * sideOffset) + forward * rayLength, 1.0f, { 0, 1, 0, 1 }, { 0, 1, 0, 1 });

    Physics::RaycastPayload rightRay = Physics::Raycast(start + right * sideOffset, forward, rayLength, 2);
    //Debug::DrawLine(start + right * sideOffset, (start + right * sideOffset) + forward * rayLength, 1.0f, { 0, 0, 1, 1 }, { 0, 0, 1, 1 });

    // Return the closest valid hit
    std::vector<Physics::RaycastPayload> hits = { centerRay, leftRay, rightRay };
    Physics::RaycastPayload closestHit;
    float closestDistance = std::numeric_limits<float>::max();

    for (const auto& hit : hits)
    {
        if (hit.hit && hit.hitDistance < closestDistance)
        {
            closestDistance = hit.hitDistance;
            closestHit = hit;
        }
    }
    if (closestHit.hit)
    {
        
        closestHit.hitNormal = glm::normalize(closestHit.hitNormal);
        glm::vec3 incoming = glm::normalize(this->linearVelocity);



        if (glm::dot(closestHit.hitNormal, -incoming) < 0.0f)
        {
            closestHit.hitNormal = -closestHit.hitNormal;
        }
    }
   
    return closestHit;
}

void Ball::SimulateCollision(glm::vec3 normal)
{
    this->linearVelocity = glm::reflect(this->linearVelocity, normal);

    // Apply a small velocity damping to avoid endless bouncing
    this->linearVelocity *= 0.95f;

    // Nudge slightly away from the surface to avoid sticking
    this->position += normal * 0.01f;

    // Update transform
    this->transform[3] = glm::vec4(this->position, 1.0f);
}
}