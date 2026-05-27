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

  // Restore jump ability only on the frame we land (air -> ground edge)
  if (!wasGrounded && grounded) {
    canJump = true;
  }
  wasGrounded = grounded;

  // Horizontal movement with forces
  float deltaTime = GetFrameTime();
  float currentControl = grounded ? 1.0f : airControl;
  float moveForce = 1500.0f;
  float maxImpulse = moveForce * currentControl * deltaTime;

  if (moveLeft) {
      facingDirection = -1;
      if (velocity.x > -moveSpeed) {
          float needed = -moveSpeed - velocity.x;
          float impulse = std::max(needed, -maxImpulse);
          object->ApplyImpulse({impulse, 0});
      }
  } else if (moveRight) {
      facingDirection = 1;
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

  // Jumping — single jump only (canJump resets on landing)
  if (jump && grounded && canJump) {
    object->ApplyImpulse({0, -jumpForce});
    canJump = false;
  }
}

void PlatformerController::ClampVelocityX() {
    // Deprecated
}

bool PlatformerController::IsGrounded() const {
  if (!object)
    return false;

  // If moving upwards quickly, definitely not grounded
  if (object->GetVelocity().y < -10.0f)
    return false;

  Rectangle aabb = object->GetAABB();
  
  // Cast rays down from bottom-left, bottom-center, and bottom-right
  // Start slightly inside the player to ensure we cross the floor's top edge
  float startY = aabb.y + aabb.height - 2.0f;
  Vector2 origins[3] = {
    { aabb.x + 2.0f, startY },
    { aabb.x + aabb.width / 2.0f, startY },
    { aabb.x + aabb.width - 2.0f, startY }
  };
  
  Vector2 direction = { 0.0f, 1.0f }; // Straight down
  float checkDistance = 10.0f; // Look from inside player to slightly below feet

  auto& physics = Graphic2D::Physics::Instance();
  for (int i = 0; i < 3; i++) {
    auto hits = physics.RaycastAll(origins[i], direction, checkDistance);
    for (const auto& hit : hits) {
      if (hit.hit && hit.object && hit.object != object) {
        if (hit.object->IsCollidable() && !hit.object->IsTrigger()) {
            return true;
        }
      }
    }
  }

  return false;
}

} // namespace Platformer
} // namespace Fumbo
