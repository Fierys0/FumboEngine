#pragma once
// Fumbo Top-Down Extension
// Include this for top-down game helpers (Stardew Valley, Zelda style)

#include "../fumbo.hpp"
#include "character_controller.hpp" // For ObjectConfig
#include "topdown/topdown_controller.hpp"

namespace Fumbo {
namespace TopDown {

using namespace Graphic2D;

// Create a wall/obstacle for top-down games
inline Object *CreateWall(Vector2 position, Vector2 size,
                          Color color = DARKGRAY) {
  Graphic2D::ObjectConfig config;
  config.position = position;
  config.size = size;
  config.color = color;
  config.bodyType = BodyType::Static;
  config.friction = 0.0f;
  config.restitution = 0.0f;
  config.gravityScale = 0.0f; // No gravity in top-down

  Object *wall = new Object();
  Graphic2D::ConfigureObject(wall, config);
  Physics::Instance().AddObject(wall);

  return wall;
}

// Create a top-down character with proper settings
inline Object *CreateCharacter(Vector2 position, Vector2 size,
                               Color color = BLUE) {
  Graphic2D::ObjectConfig config;
  config.position = position;
  config.size = size;
  config.color = color;
  config.mass = 1.0f;
  config.friction = 0.0f;
  config.restitution = 0.0f;
  config.gravityScale = 0.0f; // CRITICAL: No gravity for top-down!

  Object *character = new Object();
  Graphic2D::ConfigureObject(character, config);
  Physics::Instance().AddObject(character);

  return character;
}

// Create a top-down controller
inline TopDown::TopDownController *CreateController(Object *character,
                                                    float moveSpeed = 250.0f) {
  auto controller = new TopDown::TopDownController(character);
  controller->SetMoveSpeed(moveSpeed);
  return controller;
}

} // namespace TopDown
} // namespace Fumbo
