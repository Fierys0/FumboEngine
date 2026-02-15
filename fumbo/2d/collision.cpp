#include "../../fumbo.hpp"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Fumbo {
namespace Graphic2D {
namespace Collision {

// Main collision dispatcher
CollisionContact CheckCollision(const Object *a, const Object *b) {
  CollisionContact contact;
  contact.hasCollision = false;

  if (!a || !b)
    return contact;

  // Check collision layers
  if (!a->GetCollisionLayers().CanCollideWith(b->GetCollisionLayers())) {
    return contact;
  }

  ShapeType typeA = a->GetShapeType();
  ShapeType typeB = b->GetShapeType();

  // Rectangle vs Rectangle
  if (typeA == ShapeType::Rectangle && typeB == ShapeType::Rectangle) {
    return RectangleVsRectangle(a->GetPosition(), a->GetWidth(), a->GetHeight(),
                                a->GetRotation(), b->GetPosition(),
                                b->GetWidth(), b->GetHeight(), b->GetRotation());
  }

  // Circle vs Circle
  if (typeA == ShapeType::Circle && typeB == ShapeType::Circle) {
    return CircleVsCircle(a->GetPosition(), a->GetRadius(), b->GetPosition(),
                          b->GetRadius());
  }

  // Rectangle vs Circle
  if (typeA == ShapeType::Rectangle && typeB == ShapeType::Circle) {
    return RectangleVsCircle(a->GetPosition(), a->GetWidth(), a->GetHeight(),
                             a->GetRotation(), b->GetPosition(), b->GetRadius());
  }

  // Circle vs Rectangle (swap)
  if (typeA == ShapeType::Circle && typeB == ShapeType::Rectangle) {
    contact = RectangleVsCircle(b->GetPosition(), b->GetWidth(), b->GetHeight(),
                                b->GetRotation(), a->GetPosition(),
                                a->GetRadius());
    contact.normal = Vector2Scale(contact.normal, -1.0f); // Flip normal
    return contact;
  }

  // Polygon/Triangle vs Polygon/Triangle
  if ((typeA == ShapeType::Polygon || typeA == ShapeType::Triangle) &&
      (typeB == ShapeType::Polygon || typeB == ShapeType::Triangle)) {
    return PolygonVsPolygon(a->GetVertices(), b->GetVertices());
  }

  // Polygon vs Circle
  if ((typeA == ShapeType::Polygon || typeA == ShapeType::Triangle) &&
      typeB == ShapeType::Circle) {
    return PolygonVsCircle(a->GetVertices(), b->GetPosition(), b->GetRadius());
  }

  // Circle vs Polygon (swap)
  if (typeA == ShapeType::Circle &&
      (typeB == ShapeType::Polygon || typeB == ShapeType::Triangle)) {
    contact = PolygonVsCircle(b->GetVertices(), a->GetPosition(), a->GetRadius());
    contact.normal = Vector2Scale(contact.normal, -1.0f);
    return contact;
  }

  return contact;
}

// Rectangle vs Rectangle collision (AABB when no rotation, SAT when rotated)
CollisionContact RectangleVsRectangle(Vector2 posA, float widthA, float heightA,
                                      float rotA, Vector2 posB, float widthB,
                                      float heightB, float rotB) {
  CollisionContact contact;
  contact.hasCollision = false;

  // Simple AABB check for non-rotated rectangles
  if (rotA == 0.0f && rotB == 0.0f) {
    Rectangle rectA = {posA.x - widthA / 2, posA.y - heightA / 2, widthA,
                       heightA};
    Rectangle rectB = {posB.x - widthB / 2, posB.y - heightB / 2, widthB,
                       heightB};

    if (CheckCollisionRecs(rectA, rectB)) {
      contact.hasCollision = true;

      // Calculate penetration and normal
      Vector2 delta = Vector2Subtract(posB, posA);
      float overlapX = (widthA + widthB) / 2 - fabsf(delta.x);
      float overlapY = (heightA + heightB) / 2 - fabsf(delta.y);

      if (overlapX < overlapY) {
        contact.penetration = overlapX;
        contact.normal = {delta.x > 0 ? 1.0f : -1.0f, 0.0f};
      } else {
        contact.penetration = overlapY;
        contact.normal = {0.0f, delta.y > 0 ? 1.0f : -1.0f};
      }

      contact.point = Vector2Add(posA, Vector2Scale(delta, 0.5f));
    }
  } else {
    // Use SAT for rotated rectangles
    std::vector<Vector2> vertsA =
        GetRectangleVertices(posA, widthA, heightA, rotA);
    std::vector<Vector2> vertsB =
        GetRectangleVertices(posB, widthB, heightB, rotB);
    contact = PolygonVsPolygon(vertsA, vertsB);
  }

  return contact;
}

// Circle vs Circle collision
CollisionContact CircleVsCircle(Vector2 posA, float radiusA, Vector2 posB,
                                float radiusB) {
  CollisionContact contact;
  contact.hasCollision = false;

  Vector2 delta = Vector2Subtract(posB, posA);
  float distSq = delta.x * delta.x + delta.y * delta.y;
  float radiusSum = radiusA + radiusB;

  if (distSq < radiusSum * radiusSum) {
    contact.hasCollision = true;
    float dist = sqrtf(distSq);

    if (dist > 0.0001f) {
      contact.normal = Vector2Scale(delta, 1.0f / dist);
    } else {
      contact.normal = {1.0f, 0.0f};
    }

    contact.penetration = radiusSum - dist;
    contact.point = Vector2Add(
        posA, Vector2Scale(contact.normal, radiusA - contact.penetration / 2));
  }

  return contact;
}

// Rectangle vs Circle collision
CollisionContact RectangleVsCircle(Vector2 rectPos, float width, float height,
                                   float rotation, Vector2 circlePos,
                                   float radius) {
  CollisionContact contact;
  contact.hasCollision = false;

  // Transform circle to rectangle's local space
  Vector2 localCircle = Vector2Subtract(circlePos, rectPos);
  if (rotation != 0.0f) {
    localCircle = RotatePoint(localCircle, {0, 0}, -rotation);
  }

  // Find closest point on rectangle to circle
  float halfWidth = width / 2;
  float halfHeight = height / 2;
  Vector2 closest;
  closest.x = fmaxf(-halfWidth, fminf(halfWidth, localCircle.x));
  closest.y = fmaxf(-halfHeight, fminf(halfHeight, localCircle.y));

  // Check if closest point is inside circle
  Vector2 delta = Vector2Subtract(localCircle, closest);
  float distSq = delta.x * delta.x + delta.y * delta.y;

  if (distSq < radius * radius) {
    contact.hasCollision = true;
    float dist = sqrtf(distSq);

    if (dist > 0.0001f) {
      contact.normal = Vector2Scale(delta, 1.0f / dist);
      if (rotation != 0.0f) {
        contact.normal = RotatePoint(contact.normal, {0, 0}, rotation);
      }
    } else {
      // Circle center inside rectangle
      contact.normal = {0.0f, -1.0f};
    }

    contact.penetration = radius - dist;

    // Transform closest point back to world space
    if (rotation != 0.0f) {
      closest = RotatePoint(closest, {0, 0}, rotation);
    }
    contact.point = Vector2Add(rectPos, closest);
  }

  return contact;
}

// Polygon vs Polygon collision using SAT (Separating Axis Theorem)
CollisionContact PolygonVsPolygon(const std::vector<Vector2> &vertsA,
                                  const std::vector<Vector2> &vertsB) {
  CollisionContact contact;
  contact.hasCollision = false;

  if (vertsA.size() < 3 || vertsB.size() < 3)
    return contact;

  float minPenetration = std::numeric_limits<float>::max();
  Vector2 collisionNormal = {0, 0};

  // Test all axes from both polygons
  auto testAxes = [&](const std::vector<Vector2> &verts,
                      const std::vector<Vector2> &other) -> bool {
    for (size_t i = 0; i < verts.size(); i++) {
      Vector2 p1 = verts[i];
      Vector2 p2 = verts[(i + 1) % verts.size()];
      Vector2 edge = Vector2Subtract(p2, p1);
      Vector2 axis = Vector2Normalize({-edge.y, edge.x}); // Perpendicular

      // Project both polygons onto axis
      float minA = std::numeric_limits<float>::max();
      float maxA = std::numeric_limits<float>::lowest();
      for (const auto &v : verts) {
        float proj = Vector2DotProduct(v, axis);
        minA = fminf(minA, proj);
        maxA = fmaxf(maxA, proj);
      }

      float minB = std::numeric_limits<float>::max();
      float maxB = std::numeric_limits<float>::lowest();
      for (const auto &v : other) {
        float proj = Vector2DotProduct(v, axis);
        minB = fminf(minB, proj);
        maxB = fmaxf(maxB, proj);
      }

      // Check for separation
      if (maxA < minB || maxB < minA) {
        return false; // Separating axis found
      }

      // Calculate penetration
      float penetration = fminf(maxA - minB, maxB - minA);
      if (penetration < minPenetration) {
        minPenetration = penetration;
        collisionNormal = axis;
      }
    }
    return true;
  };

  if (!testAxes(vertsA, vertsB) || !testAxes(vertsB, vertsA)) {
    return contact;
  }

  // Collision detected
  contact.hasCollision = true;
  contact.penetration = minPenetration;
  contact.normal = collisionNormal;

  // Calculate contact point (average of overlapping vertices)
  Vector2 contactSum = {0, 0};
  int contactCount = 0;
  for (const auto &v : vertsA) {
    // Simple heuristic: vertices of A inside B's bounds
    Rectangle boundsB = GetBoundingBox(vertsB);
    if (CheckCollisionPointRec(v, boundsB)) {
      contactSum = Vector2Add(contactSum, v);
      contactCount++;
    }
  }
  if (contactCount > 0) {
    contact.point = Vector2Scale(contactSum, 1.0f / contactCount);
  }

  return contact;
}

// Polygon vs Circle collision
CollisionContact PolygonVsCircle(const std::vector<Vector2> &verts,
                                 Vector2 circlePos, float radius) {
  CollisionContact contact;
  contact.hasCollision = false;

  if (verts.size() < 3)
    return contact;

  // Find closest edge to circle
  float minDistSq = std::numeric_limits<float>::max();
  Vector2 closestPoint = {0, 0};

  for (size_t i = 0; i < verts.size(); i++) {
    Vector2 p1 = verts[i];
    Vector2 p2 = verts[(i + 1) % verts.size()];

    // Find closest point on edge to circle center
    Vector2 edge = Vector2Subtract(p2, p1);
    Vector2 toCircle = Vector2Subtract(circlePos, p1);
    float edgeLengthSq = Vector2DotProduct(edge, edge);
    float t = Vector2DotProduct(toCircle, edge) / edgeLengthSq;
    t = fmaxf(0.0f, fminf(1.0f, t));

    Vector2 pointOnEdge = Vector2Add(p1, Vector2Scale(edge, t));
    Vector2 delta = Vector2Subtract(circlePos, pointOnEdge);
    float distSq = Vector2DotProduct(delta, delta);

    if (distSq < minDistSq) {
      minDistSq = distSq;
      closestPoint = pointOnEdge;
    }
  }

  // Check if circle overlaps
  if (minDistSq < radius * radius) {
    contact.hasCollision = true;
    float dist = sqrtf(minDistSq);
    Vector2 delta = Vector2Subtract(circlePos, closestPoint);

    if (dist > 0.0001f) {
      contact.normal = Vector2Scale(delta, 1.0f / dist);
    } else {
      contact.normal = {0.0f, 1.0f};
    }

    contact.penetration = radius - dist;
    contact.point = closestPoint;
  }

  return contact;
}

// Helper: Rotate point around origin
Vector2 RotatePoint(Vector2 point, Vector2 origin, float angle) {
  float s = sinf(angle * DEG2RAD);
  float c = cosf(angle * DEG2RAD);

  point = Vector2Subtract(point, origin);

  float xnew = point.x * c - point.y * s;
  float ynew = point.x * s + point.y * c;

  return Vector2Add({xnew, ynew}, origin);
}

// Helper: Get rectangle vertices
std::vector<Vector2> GetRectangleVertices(Vector2 pos, float width,
                                          float height, float rotation) {
  float halfW = width / 2;
  float halfH = height / 2;

  std::vector<Vector2> verts = {
      {-halfW, -halfH}, {halfW, -halfH}, {halfW, halfH}, {-halfW, halfH}};

  // Rotate and translate
  for (auto &v : verts) {
    if (rotation != 0.0f) {
      v = RotatePoint(v, {0, 0}, rotation);
    }
    v = Vector2Add(v, pos);
  }

  return verts;
}

// Helper: Get bounding box of vertices
Rectangle GetBoundingBox(const std::vector<Vector2> &vertices) {
  if (vertices.empty())
    return {0, 0, 0, 0};

  float minX = vertices[0].x, maxX = vertices[0].x;
  float minY = vertices[0].y, maxY = vertices[0].y;

  for (const auto &v : vertices) {
    minX = fminf(minX, v.x);
    maxX = fmaxf(maxX, v.x);
    minY = fminf(minY, v.y);
    maxY = fmaxf(maxY, v.y);
  }

  return {minX, minY, maxX - minX, maxY - minY};
}

// Helper: Line intersection test
bool LineIntersection(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
                     Vector2 *intersection) {
  float dx1 = p2.x - p1.x;
  float dy1 = p2.y - p1.y;
  float dx2 = p4.x - p3.x;
  float dy2 = p4.y - p3.y;

  float denom = dx1 * dy2 - dy1 * dx2;
  if (fabsf(denom) < 0.0001f)
    return false;

  float t = ((p3.x - p1.x) * dy2 - (p3.y - p1.y) * dx2) / denom;
  float u = ((p3.x - p1.x) * dy1 - (p3.y - p1.y) * dx1) / denom;

  if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
    if (intersection) {
      intersection->x = p1.x + t * dx1;
      intersection->y = p1.y + t * dy1;
    }
    return true;
  }

  return false;
}

} // namespace Collision
} // namespace Graphic2D
} // namespace Fumbo
