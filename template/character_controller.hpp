#pragma once
#include "../fumbo.hpp"
#include <cmath>

namespace Fumbo {
namespace Graphic2D {

// Platformer-specific controller for side-scrolling games
class PlatformerController {
public:
  PlatformerController(Object *controlledObject);

  // Configuration
  void SetMoveSpeed(float speed) { moveSpeed = speed; }
  void SetJumpForce(float force) { jumpForce = force; }
  void SetAirControl(float control) { airControl = control; }

  // Call this every frame with input
  void Update(bool moveLeft, bool moveRight, bool jump);

  // Utility
  bool IsGrounded() const;

private:
  Object *object;

  // Movement settings
  float moveSpeed = 350.0f;
  float jumpForce = 600.0f;
  float airControl = 0.3f;
  float groundedThreshold = 50.0f;
};

// Top-down controller for games like Stardew Valley, Zelda
class TopDownController {
public:
  TopDownController(Object *controlledObject);

  // Configuration
  void SetMoveSpeed(float speed) { moveSpeed = speed; }

  // 4-directional movement
  void Update(bool moveLeft, bool moveRight, bool moveUp, bool moveDown);

private:
  Object *object;

  // Movement settings
  float moveSpeed = 250.0f;
};

// Configuration struct for easy object setup
struct ObjectConfig {
  // Shape
  Vector2 size = {100, 100};

  // Physics
  float mass = 1.0f;
  float friction = 0.0f;
  float restitution = 0.0f;
  float gravityScale = 1.0f;
  BodyType bodyType = BodyType::Dynamic;

  // Visual
  Color color = WHITE;
  Texture2D texture = {0};
  bool hasTexture = false;

  // Position
  Vector2 position = {0, 0};
};

// Apply configuration to an object
inline void ConfigureObject(Object *obj, const ObjectConfig &config) {
  obj->SetRectangle(config.size.x, config.size.y);
  obj->SetPosition(config.position);
  obj->SetMass(config.mass);
  obj->SetFriction(config.friction);
  obj->SetRestitution(config.restitution);
  obj->SetGravityScale(config.gravityScale);
  obj->SetBodyType(config.bodyType);
  obj->SetColor(config.color);

  if (config.hasTexture && config.texture.id != 0) {
    obj->SetTexture(config.texture);
  }
}

} // namespace Graphic2D
} // namespace Fumbo
