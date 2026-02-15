#include "character_controller.hpp"
#include <cmath>

namespace Fumbo {
namespace Graphic2D {

// PlatformerController

// PlatformerController

PlatformerController::PlatformerController(Object *controlledObject)
    : object(controlledObject) {}

void PlatformerController::Update(bool moveLeft, bool moveRight, bool jump) {
  if (!object)
    return;

  Vector2 velocity = object->GetVelocity();
  bool grounded = IsGrounded();

  // Horizontal movement with forces
  float deltaTime = GetFrameTime();
  float currentControl = grounded ? 1.0f : airControl;
  float moveForce = 1500.0f;
  float maxImpulse = moveForce * currentControl * deltaTime;

  if (moveLeft) {
      // Target velocity is -moveSpeed
      // We only apply force if we are to the right of the target velocity (i.e. moving slower left, or moving right)
      if (velocity.x > -moveSpeed) {
          float needed = -moveSpeed - velocity.x;
          // Clamp the impulse so we don't overshoot
          float impulse = std::max(needed, -maxImpulse);
          object->ApplyImpulse({impulse, 0});
      }
  } else if (moveRight) {
      if (velocity.x < moveSpeed) {
          float needed = moveSpeed - velocity.x;
          float impulse = std::min(needed, maxImpulse);
          object->ApplyImpulse({impulse, 0});
      }
  } else {
    // Damping when no input
    if (grounded) {
      if (abs(velocity.x) > 1.0f) {
        float dampingForce = -velocity.x * 20.0f;
        object->ApplyImpulse({dampingForce * deltaTime, 0});
      } else {
        object->SetVelocity({0, velocity.y});
      }
    }
  }

  // Jumping
  if (jump && grounded) {
    object->ApplyImpulse({0, -jumpForce});
  }
}


bool PlatformerController::IsGrounded() const {
  if (!object)
    return false;
  return abs(object->GetVelocity().y) < groundedThreshold;
}

// TopDownController

TopDownController::TopDownController(Object *controlledObject)
    : object(controlledObject) {}

void TopDownController::Update(bool moveLeft, bool moveRight, bool moveUp,
                               bool moveDown) {
  if (!object)
    return;

  Vector2 velocity = object->GetVelocity();
  float moveForce = 2000.0f;
  float deltaTime = GetFrameTime();
  float maxImpulse = moveForce * deltaTime;

  // 4-directional movement
  Vector2 inputDirection = {0, 0};
  if (moveLeft)
    inputDirection.x -= 1;
  if (moveRight)
    inputDirection.x += 1;
  if (moveUp)
    inputDirection.y -= 1;
  if (moveDown)
    inputDirection.y += 1;

  // Normalize
  float magnitude = sqrt(inputDirection.x * inputDirection.x + inputDirection.y * inputDirection.y);
  if (magnitude > 0) {
    inputDirection.x /= magnitude;
    inputDirection.y /= magnitude;
    
    Vector2 targetVelocity = {inputDirection.x * moveSpeed, inputDirection.y * moveSpeed};
    Vector2 currentVelocity = object->GetVelocity();
    
    // Independent Axis Control to reach target velocity
    // This allows for smooth diagonal movement and speed limiting
    
    // X Axis
    float diffX = targetVelocity.x - currentVelocity.x;

    float impulseX = 0;
    if (abs(diffX) > 0.01f) {
        impulseX = diffX > 0 ? std::min(diffX, maxImpulse) : std::max(diffX, -maxImpulse);
    }

    // Y Axis
    float diffY = targetVelocity.y - currentVelocity.y;
    float impulseY = 0;
    if (abs(diffY) > 0.01f) {
        impulseY = diffY > 0 ? std::min(diffY, maxImpulse) : std::max(diffY, -maxImpulse);
    }
    object->ApplyImpulse({impulseX, impulseY});

  } else {
    // Strong damping when no input
    if (abs(velocity.x) > 1.0f || abs(velocity.y) > 1.0f) {
      float dampingForce = 20.0f;
      object->ApplyImpulse(
          {-velocity.x * dampingForce * deltaTime, -velocity.y * dampingForce * deltaTime});
    } else {
      object->SetVelocity({0, 0});
    }
  }
}

} // namespace Graphic2D
} // namespace Fumbo
