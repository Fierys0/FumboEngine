#pragma once
#include "raylib.h"
#include <cstdint>
#include <vector>

namespace Fumbo {
namespace Graphic2D {

// Shape types supported by the physics engine
enum class ShapeType {
  Rectangle,
  Circle,
  Triangle,
  Polygon,
  Line
};

// Body type determines physics behavior
enum class BodyType {
  Static,  // Doesn't move, but can collide
  Dynamic  // Affected by gravity and forces
};

// Collision data for contact resolution
struct CollisionContact {
  Vector2 point;        // Contact point in world space
  Vector2 normal;       // Collision normal (from A to B)
  float penetration;    // Penetration depth
  bool hasCollision;    // Whether collision occurred
};

// Forward declaration
class Object;

// Collision detection functions
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

} // namespace Graphic2D
} // namespace Fumbo
