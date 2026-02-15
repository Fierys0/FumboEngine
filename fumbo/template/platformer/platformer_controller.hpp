#pragma once
#include "../../2d/object.hpp"
#include "../../2d/physics.hpp"
#include "raylib.h"

namespace Fumbo {
namespace Platformer {

// Platformer-specific controller for side-scrolling games
class PlatformerController {
public:
  PlatformerController(Graphic2D::Object *controlledObject);

  // Configuration
  void SetMoveSpeed(float speed) { moveSpeed = speed; }
  void SetJumpForce(float force) { jumpForce = force; }
  void SetAirControl(float control) { airControl = control; }

  // Call this every frame with input
  void Update(bool moveLeft, bool moveRight, bool jump);

  // Utility
  bool IsGrounded() const;

private:
  Graphic2D::Object *object;

  // Movement settings
  float moveSpeed = 350.0f;
  float jumpForce = 600.0f;
  float airControl = 1.0f;
  float groundedThreshold = 50.0f;

  void ClampVelocityX();
};

} // namespace Platformer
} // namespace Fumbo
