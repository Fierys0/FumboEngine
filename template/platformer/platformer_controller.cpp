#include "platformer_controller.hpp"
#include <cmath>

namespace Fumbo {
namespace Platformer {

PlatformerController::PlatformerController(Graphic2D::Object *controlledObject)
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
      if (velocity.x > -moveSpeed) {
          float needed = -moveSpeed - velocity.x;
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

void PlatformerController::ClampVelocityX() {
    // Deprecated
}

bool PlatformerController::IsGrounded() const {
  if (!object)
    return false;
  return abs(object->GetVelocity().y) < groundedThreshold;
}

} // namespace Platformer
} // namespace Fumbo
