#include "topdown_controller.hpp"
#include <cmath>

namespace Fumbo {
namespace TopDown {

TopDownController::TopDownController(Graphic2D::Object* controlledObject)
    : object(controlledObject) {}

void TopDownController::Update(bool moveLeft, bool moveRight, bool moveUp, bool moveDown) {
  if (!object) return;
  
  Vector2 velocity = object->GetVelocity();
  float moveForce = 2000.0f;
  
  // 4-directional movement
  Vector2 inputDirection = {0, 0};
  if (moveLeft) inputDirection.x -= 1;
  if (moveRight) inputDirection.x += 1;
  if (moveUp) inputDirection.y -= 1;
  if (moveDown) inputDirection.y += 1;
  
  // Normalize diagonal movement
  float magnitude = sqrt(inputDirection.x * inputDirection.x + inputDirection.y * inputDirection.y);
  if (magnitude > 0) {
    inputDirection.x /= magnitude;
    inputDirection.y /= magnitude;
  }
  
  float deltaTime = GetFrameTime();
  float maxImpulse = moveForce * deltaTime;

  if (magnitude > 0) {
    Vector2 targetVelocity = {inputDirection.x * moveSpeed, inputDirection.y * moveSpeed};
    
    // Independent Axis Control to reach target velocity
    // X Axis
    float diffX = targetVelocity.x - velocity.x;
    float impulseX = 0;
    // Only apply force if there is a significant difference
    if (abs(diffX) > 0.1f) {
        impulseX = diffX > 0 ? std::min(diffX, maxImpulse) : std::max(diffX, -maxImpulse);
    }

    // Y Axis
    float diffY = targetVelocity.y - velocity.y;
    float impulseY = 0;
    if (abs(diffY) > 0.1f) {
        impulseY = diffY > 0 ? std::min(diffY, maxImpulse) : std::max(diffY, -maxImpulse);
    }
    
    object->ApplyImpulse({impulseX, impulseY});

  } else {
    // Strong damping when no input
    if (abs(velocity.x) > 1.0f || abs(velocity.y) > 1.0f) {
      float dampingForce = 20.0f;
      object->ApplyImpulse({-velocity.x * dampingForce * deltaTime, -velocity.y * dampingForce * deltaTime});
    } else {
      object->SetVelocity({0, 0});
    }
  }

}


} // namespace TopDown
} // namespace Fumbo
