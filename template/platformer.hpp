#pragma once
// Fumbo Platformer Extension
// Include this for platformer-specific helpers and controllers

#include "../fumbo.hpp"
#include "character_controller.hpp" // For ObjectConfig
#include "platformer/platformer_controller.hpp"

namespace Fumbo {
namespace Platformer {

using namespace Graphic2D;

// Create a static platform
inline Object *CreatePlatform(Vector2 position, Vector2 size,
                              Color color = GRAY) {
  Graphic2D::ObjectConfig config;
  config.position = position;
  config.size = size;
  config.color = color;
  config.bodyType = BodyType::Static;
  config.friction = 0.2f;
  config.restitution = 0.0f;

  Object *platform = new Object();
  Graphic2D::ConfigureObject(platform, config);
  Physics::Instance().AddObject(platform);

  return platform;
}

// Create a platformer character with good defaults
inline Object *CreateCharacter(Vector2 position, Vector2 size,
                               Color color = RED) {
  Graphic2D::ObjectConfig config;
  config.position = position;
  config.size = size;
  config.color = color;
  config.mass = 1.0f;
  config.friction = 0.0f;
  config.restitution = 0.0f;
  config.gravityScale = 1.0f;

  Object *character = new Object();
  Graphic2D::ConfigureObject(character, config);
  Physics::Instance().AddObject(character);

  return character;
}

// Create a platformer controller
inline Platformer::PlatformerController *
CreateController(Object *character, float moveSpeed = 350.0f,
                 float jumpForce = 600.0f) {
  auto controller = new Platformer::PlatformerController(character);
  controller->SetMoveSpeed(moveSpeed);
  controller->SetJumpForce(jumpForce);
  return controller;
}

} // namespace Platformer
} // namespace Fumbo
