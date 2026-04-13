#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cmath>
#include <cstdint>

namespace Fumbo {
namespace Graphic2D {

// Shape types supported by the physics engine
enum class ShapeType { Rectangle, Circle, Triangle, Polygon, Line };

// Body type determines physics behavior
enum class BodyType {
  Static, // Doesn't move, but can collide
  Dynamic // Affected by gravity and forces
};

// Collision data for contact resolution
struct CollisionContact {
  Vector2 point;     // Contact point in world space
  Vector2 normal;    // Collision normal (from A to B)
  float penetration; // Penetration depth
  bool hasCollision; // Whether collision occurred
};

// Forward declaration
class Object;

// === FUMBO Collision
namespace Collision {

// Main collision detection dispatcher
CollisionContact CheckCollision(const Object *a, const Object *b);

// Shape-specific collision checks
CollisionContact RectangleVsRectangle(Vector2 posA, float widthA, float heightA,
                                      float rotA, Vector2 posB, float widthB,
                                      float heightB, float rotB);

CollisionContact CircleVsCircle(Vector2 posA, float radiusA, Vector2 posB,
                                float radiusB);

CollisionContact RectangleVsCircle(Vector2 rectPos, float width, float height,
                                   float rotation, Vector2 circlePos,
                                   float radius);

CollisionContact PolygonVsPolygon(const std::vector<Vector2> &vertsA,
                                  const std::vector<Vector2> &vertsB);

CollisionContact PolygonVsCircle(const std::vector<Vector2> &verts,
                                 Vector2 circlePos, float radius);

// Helper functions
Vector2 RotatePoint(Vector2 point, Vector2 origin, float angle);
std::vector<Vector2> GetRectangleVertices(Vector2 pos, float width,
                                          float height, float rotation);
Rectangle GetBoundingBox(const std::vector<Vector2> &vertices);
bool LineIntersection(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
                      Vector2 *intersection = nullptr);

} // namespace Collision

// Collision layer management (32 layers max)
class CollisionLayers {
public:
  static constexpr int MAX_LAYERS = 32;

  CollisionLayers() : layerMask(0xFFFFFFFF) {} // Collide with all by default

  void SetLayer(int layer) { currentLayer = layer; }
  int GetLayer() const { return currentLayer; }

  void SetMask(uint32_t mask) { layerMask = mask; }
  uint32_t GetMask() const { return layerMask; }

  void EnableLayer(int layer) { layerMask |= (1 << layer); }
  void DisableLayer(int layer) { layerMask &= ~(1 << layer); }

  bool CanCollideWith(const CollisionLayers &other) const {
    return (layerMask & (1 << other.currentLayer)) != 0;
  }

private:
  int currentLayer = 0;
  uint32_t layerMask;
};

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

  void SetCollisionLayers(const CollisionLayers &layers) {
    collisionLayers = layers;
  }
  CollisionLayers &GetCollisionLayers() { return collisionLayers; }
  const CollisionLayers &GetCollisionLayers() const { return collisionLayers; }

  // Check if this object is currently colliding with another
  bool IsCollidingWith(const Object *other) const;

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
  void SetTexture(Texture2D tex) {
    texture = tex;
    hasTexture = true;
  }
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
  float width, height;           // Rectangle
  float radius;                  // Circle
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
  bool isCollidable = true; // Can this object collide?
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
  void ResolveCollision(Object *objectA, Object *objectB,
                        const CollisionContact &contact);
};

} // namespace Graphic2D
} // namespace Fumbo
