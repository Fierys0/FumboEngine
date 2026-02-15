#pragma once
#include "object.hpp"
#include "raylib.h"
#include <memory>
#include <vector>

namespace Fumbo {
namespace Graphic2D {

// Raycast result structure
struct RaycastHit {
  Object *object;
  Vector2 point;
  Vector2 normal;
  float distance;
  bool hit;
};

// Physics manager (Singleton)
class Physics {
public:
  static Physics &Instance() {
    static Physics instance;
    return instance;
  }

  // Global physics settings
  void SetGravity(Vector2 newGravity) { gravity = newGravity; }
  Vector2 GetGravity() const { return gravity; }

  void SetFixedTimeStep(float timeStep) { fixedTimeStep = 1.0f / timeStep; }
  float GetFixedTimeStep() const { return fixedTimeStep; }

  void SetIterations(int newIterations) { iterations = newIterations; }
  int GetIterations() const { return iterations; }

  // Object management
  void AddObject(Object *object);
  void RemoveObject(Object *object);
  void Clear();

  const std::vector<Object *> &GetObjects() const { return objects; }

  // Physics simulation
  void Update(float deltaTime);

  // Raycasting
  RaycastHit Raycast(Vector2 origin, Vector2 direction, float maxDistance);
  std::vector<RaycastHit> RaycastAll(Vector2 origin, Vector2 direction,
                                     float maxDistance);

  // Debug rendering
  void SetDebugDraw(bool enabled) { debugDraw = enabled; }
  bool IsDebugDrawEnabled() const { return debugDraw; }
  void DrawDebug() const;

private:
  Physics();
  ~Physics() = default;
  Physics(const Physics &) = delete;
  Physics &operator=(const Physics &) = delete;

  Vector2 gravity;
  float fixedTimeStep;
  float accumulator;
  int iterations;
  bool debugDraw;

  std::vector<Object *> objects;

  // Physics step
  void Step(float deltaTime);
  void ApplyGravity(float deltaTime);
  void ResolveCollisions();
  void ResolveCollision(Object *objectA, Object *objectB, const CollisionContact &contact);
};

} // namespace Graphic2D
} // namespace Fumbo
