#pragma once
#include "collision.hpp"
#include "raylib.h"
#include <cmath>
#include <vector>

namespace Fumbo {
namespace Graphic2D {

// 2D Physics Object with shape and rigidbody
class Object {
public:
  Object();
  ~Object() = default;

  // ===== Shape Type Configuration =====
  void SetRectangle(float width, float height);
  void SetCircle(float radius);
  void SetTriangle(Vector2 p1, Vector2 p2, Vector2 p3);
  void SetPolygon(const std::vector<Vector2> &vertices);
  void SetLine(Vector2 start, Vector2 end);

  ShapeType GetShapeType() const { return shapeType; }

  // ===== Transform =====
  void SetPosition(Vector2 pos) { position = pos; }
  Vector2 GetPosition() const { return position; }

  void SetRotation(float rot) { rotation = rot; }
  float GetRotation() const { return rotation; }

  void SetScale(float scl) { scale = scl; }
  float GetScale() const { return scale; }

  // ===== Rigidbody Properties =====
  void SetBodyType(BodyType type) { bodyType = type; }
  BodyType GetBodyType() const { return bodyType; }

  void SetVelocity(Vector2 vel) { velocity = vel; }
  Vector2 GetVelocity() const { return velocity; }

  void SetMass(float m) { mass = fmaxf(m, 0.001f); } // Prevent zero mass
  float GetMass() const { return mass; }

  void SetFriction(float f) { friction = f; }
  float GetFriction() const { return friction; }

  void SetDrag(float d) { drag = d; }
  float GetDrag() const { return drag; }

  void SetRestitution(float r) { restitution = r; }
  float GetRestitution() const { return restitution; }

  void SetGravityScale(float gs) { gravityScale = gs; }
  float GetGravityScale() const { return gravityScale; }

  // ===== Physics Simulation =====
  void ApplyForce(Vector2 force);
  void ApplyImpulse(Vector2 impulse);
  void Update(float deltaTime);

  // ===== Collision =====
  void SetTrigger(bool trigger) { isTrigger = trigger; }
  bool IsTrigger() const { return isTrigger; }

  // Collidable toggle - if false, object passes through everything
  void SetCollidable(bool collidable) { isCollidable = collidable; }
  bool IsCollidable() const { return isCollidable; }

  void SetCollisionLayers(const CollisionLayers& layers) {
    collisionLayers = layers;
  }
  CollisionLayers &GetCollisionLayers() { return collisionLayers; }
  const CollisionLayers &GetCollisionLayers() const { return collisionLayers; }

  // Check if this object is currently colliding with another
  bool IsCollidingWith(const Object* other) const;

  // ===== Shape-Specific Getters =====
  float GetWidth() const { return width; }
  float GetHeight() const { return height; }
  float GetRadius() const { return radius; }
  std::vector<Vector2> GetVertices() const;

  // Get axis-aligned bounding box (for broadphase collision detection)
  Rectangle GetAABB() const;

  // ===== Rendering =====
  void SetColor(Color c) { color = c; }
  Color GetColor() const { return color; }

  // Texture support
  void SetTexture(Texture2D tex) { texture = tex; hasTexture = true; }
  void ClearTexture() { hasTexture = false; }
  bool HasTexture() const { return hasTexture; }

  void SetOutline(bool outline) { isOutline = outline; }
  bool IsOutline() const { return isOutline; }

  void SetThickness(float t) { thickness = t; }
  float GetThickness() const { return thickness; }

  void Render() const;

  // ===== Debug Visualization =====
  void DrawDebug() const;

private:
  // Shape properties
  ShapeType shapeType;
  float width, height;  // Rectangle
  float radius;         // Circle
  std::vector<Vector2> vertices; // Polygon/Triangle

  // Transform
  Vector2 position;
  float rotation;
  float scale;

  // Rigidbody
  BodyType bodyType;
  Vector2 velocity;
  Vector2 acceleration;
  float mass;
  float friction;     // 0 = frictionless, 1 = high friction
  float drag;         // Air resistance
  float restitution;  // Bounciness (0 = no bounce, 1 = perfect bounce)
  float gravityScale; // Multiplier for gravity effect

  // Collision
  bool isTrigger;
  bool isCollidable = true;  // Can this object collide?
  CollisionLayers collisionLayers;

  // Visual
  Color color;
  Texture2D texture;
  bool hasTexture = false;
  bool isOutline;
  float thickness;

  // Line-specific
  Vector2 lineStart;
  Vector2 lineEnd;

  // Helper to update vertices for transform
  void UpdateVertices();
};

} // namespace Graphic2D
} // namespace Fumbo
