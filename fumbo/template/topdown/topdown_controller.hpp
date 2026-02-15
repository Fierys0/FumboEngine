#pragma once
#include "../../2d/object.hpp"
#include "../../2d/physics.hpp"
#include "raylib.h"

namespace Fumbo {
namespace TopDown {

// Top-down controller for games like Stardew Valley, Zelda
class TopDownController {
public:
  TopDownController(Graphic2D::Object *controlledObject);

  // Configuration
  void SetMoveSpeed(float speed) { moveSpeed = speed; }

  // 4-directional movement
  void Update(bool moveLeft, bool moveRight, bool moveUp, bool moveDown);

private:
  Graphic2D::Object *object;

  // Movement settings
  float moveSpeed = 250.0f;
};

} // namespace TopDown
} // namespace Fumbo
