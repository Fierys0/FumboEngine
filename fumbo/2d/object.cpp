#include "../../fumbo.hpp"
#include "raymath.h"
#include <cmath>

namespace Fumbo {
namespace Graphic2D {

Object::Object()
    : shapeType(ShapeType::Rectangle), width(100.0f), height(100.0f),
      radius(50.0f), position({0, 0}), rotation(0.0f), scale(1.0f),
      bodyType(BodyType::Dynamic), velocity({0, 0}), acceleration({0, 0}),
      mass(1.0f), friction(0.3f), drag(0.01f), restitution(0.5f),
      gravityScale(1.0f), isTrigger(false), color(BLUE), isOutline(false),
      thickness(1.0f), lineStart({0, 0}), lineEnd({100, 0}) {}

// ===== Shape Configuration =====

void Object::SetRectangle(float w, float h) {
  shapeType = ShapeType::Rectangle;
  width = w;
  height = h;
}

void Object::SetCircle(float r) {
  shapeType = ShapeType::Circle;
  radius = r;
}

void Object::SetTriangle(Vector2 p1, Vector2 p2, Vector2 p3) {
  shapeType = ShapeType::Triangle;
  vertices.clear();
  vertices.push_back(p1);
  vertices.push_back(p2);
  vertices.push_back(p3);
}

void Object::SetPolygon(const std::vector<Vector2> &verts) {
  shapeType = ShapeType::Polygon;
  vertices = verts;
}

void Object::SetLine(Vector2 start, Vector2 end) {
  shapeType = ShapeType::Line;
  lineStart = start;
  lineEnd = end;
}

// ===== Physics Simulation =====

void Object::ApplyForce(Vector2 force) {
  if (bodyType == BodyType::Dynamic) {
    acceleration = Vector2Add(acceleration, Vector2Scale(force, 1.0f / mass));
  }
}

void Object::ApplyImpulse(Vector2 impulse) {
  if (bodyType == BodyType::Dynamic) {
    velocity = Vector2Add(velocity, Vector2Scale(impulse, 1.0f / mass));
  }
}

void Object::Update(float deltaTime) {
  if (bodyType == BodyType::Static)
    return;

  // Apply drag
  velocity = Vector2Scale(velocity, 1.0f - drag * deltaTime);

  // Update velocity from acceleration
  velocity = Vector2Add(velocity, Vector2Scale(acceleration, deltaTime));

  // Update position from velocity
  position = Vector2Add(position, Vector2Scale(velocity, deltaTime));

  // Reset acceleration
  acceleration = {0, 0};

  // Update vertices if needed
  if (shapeType == ShapeType::Polygon || shapeType == ShapeType::Triangle) {
    UpdateVertices();
  }
}

// ===== Shape-Specific Getters =====

std::vector<Vector2> Object::GetVertices() const {
  if (shapeType == ShapeType::Polygon || shapeType == ShapeType::Triangle) {
    // Transform vertices to world space
    std::vector<Vector2> worldVerts;
    for (const auto &v : vertices) {
      Vector2 scaled = Vector2Scale(v, scale);
      Vector2 rotated = Collision::RotatePoint(scaled, {0, 0}, rotation);
      worldVerts.push_back(Vector2Add(rotated, position));
    }
    return worldVerts;
  } else if (shapeType == ShapeType::Rectangle) {
    return Collision::GetRectangleVertices(position, width * scale,
                                           height * scale, rotation);
  }
  return {};
}

Rectangle Object::GetAABB() const {
  switch (shapeType) {
  case ShapeType::Rectangle: {
    // For non-rotated rectangles, AABB is simple
    if (rotation == 0.0f) {
      float halfW = (width * scale) / 2;
      float halfH = (height * scale) / 2;
      return {position.x - halfW, position.y - halfH, width * scale,
              height * scale};
    }
    // For rotated rectangles, get bounding box of vertices
    auto verts = GetVertices();
    return Collision::GetBoundingBox(verts);
  }

  case ShapeType::Circle: {
    float r = radius * scale;
    return {position.x - r, position.y - r, r * 2, r * 2};
  }

  case ShapeType::Triangle:
  case ShapeType::Polygon: {
    auto verts = GetVertices();
    return Collision::GetBoundingBox(verts);
  }

  case ShapeType::Line: {
    Vector2 worldStart = Vector2Add(position, lineStart);
    Vector2 worldEnd = Vector2Add(position, lineEnd);
    float minX = fminf(worldStart.x, worldEnd.x);
    float maxX = fmaxf(worldStart.x, worldEnd.x);
    float minY = fminf(worldStart.y, worldEnd.y);
    float maxY = fmaxf(worldStart.y, worldEnd.y);
    return {minX, minY, maxX - minX, maxY - minY};
  }
  }

  return {0, 0, 0, 0};
}

// ===== Rendering =====

void Object::Render() const {
  switch (shapeType) {
  case ShapeType::Rectangle: {
    if (rotation == 0.0f) {
      Rectangle rect = {position.x - (width * scale) / 2,
                        position.y - (height * scale) / 2, width * scale,
                        height * scale};

      if (hasTexture) {
        // Draw with texture
        Rectangle source = {0, 0, (float)texture.width, (float)texture.height};
        Fumbo::Graphic2D::DrawTexturePro(texture, source, rect, {0, 0}, 0,
                                         WHITE);
      } else if (isOutline) {
        Fumbo::Graphic2D::DrawRectangleLinesEx(rect, thickness, color);
      } else {
        Fumbo::Graphic2D::DrawRectangleRec(rect, color);
      }
    } else {
      Rectangle rect = {0, 0, width * scale, height * scale};
      Vector2 origin = {(width * scale) / 2, (height * scale) / 2};

      if (hasTexture) {
        // Draw rotated texture
        Rectangle dest = {position.x, position.y, width * scale,
                          height * scale};
        Rectangle source = {0, 0, (float)texture.width, (float)texture.height};
        Fumbo::Graphic2D::DrawTexturePro(texture, source, dest, origin,
                                         rotation, WHITE);
      } else if (isOutline) {
        Fumbo::Graphic2D::DrawRectanglePro(rect, origin, rotation, BLANK);
        // Draw outline manually with lines
        auto verts = GetVertices();
        for (size_t i = 0; i < verts.size(); i++) {
          Vector2 p1 = verts[i];
          Vector2 p2 = verts[(i + 1) % verts.size()];
          Fumbo::Graphic2D::DrawLineEx(p1, p2, thickness, color);
        }
      } else {
        Fumbo::Graphic2D::DrawRectanglePro(
            {position.x, position.y, width * scale, height * scale}, origin,
            rotation, color);
      }
    }
    break;
  }

  case ShapeType::Circle: {
    if (isOutline) {
      Fumbo::Graphic2D::DrawCircleLines(position.x, position.y, radius * scale,
                                        color);
    } else {
      Fumbo::Graphic2D::DrawCircleV(position, radius * scale, color);
    }
    break;
  }

  case ShapeType::Triangle: {
    if (vertices.size() >= 3) {
      auto worldVerts = GetVertices();
      if (isOutline) {
        Fumbo::Graphic2D::DrawTriangleLines(worldVerts[0], worldVerts[1],
                                            worldVerts[2], color);
      } else {
        Fumbo::Graphic2D::DrawTriangle(worldVerts[0], worldVerts[1],
                                       worldVerts[2], color);
      }
    }
    break;
  }

  case ShapeType::Polygon: {
    auto worldVerts = GetVertices();
    if (worldVerts.size() >= 3) {
      if (isOutline) {
        for (size_t i = 0; i < worldVerts.size(); i++) {
          Vector2 p1 = worldVerts[i];
          Vector2 p2 = worldVerts[(i + 1) % worldVerts.size()];
          Fumbo::Graphic2D::DrawLineEx(p1, p2, thickness, color);
        }
      } else {
        // Draw filled polygon using triangle fan
        for (size_t i = 1; i < worldVerts.size() - 1; i++) {
          Fumbo::Graphic2D::DrawTriangle(worldVerts[0], worldVerts[i],
                                         worldVerts[i + 1], color);
        }
      }
    }
    break;
  }

  case ShapeType::Line: {
    Vector2 worldStart = Vector2Add(position, lineStart);
    Vector2 worldEnd = Vector2Add(position, lineEnd);
    Fumbo::Graphic2D::DrawLineEx(worldStart, worldEnd, thickness, color);
    break;
  }
  }
}

void Object::DrawDebug() const {
  // Draw bounding box
  auto verts = GetVertices();
  if (!verts.empty()) {
    Rectangle bounds = Collision::GetBoundingBox(verts);
    Fumbo::Graphic2D::DrawRectangleLinesEx(bounds, 1.0f, YELLOW);
  } else if (shapeType == ShapeType::Circle) {
    Fumbo::Graphic2D::DrawCircleLines(position.x, position.y, radius * scale,
                                      YELLOW);
  }

  // Draw velocity vector
  if (bodyType == BodyType::Dynamic) {
    Vector2 velEnd = Vector2Add(position, Vector2Scale(velocity, 0.1f));
    Fumbo::Graphic2D::DrawLineEx(position, velEnd, 2.0f, GREEN);
    Fumbo::Graphic2D::DrawCircleV(velEnd, 4.0f, GREEN);
  }

  // Draw center point
  Fumbo::Graphic2D::DrawCircleV(position, 3.0f, RED);

  // Draw body type text
  const char *typeText = (bodyType == BodyType::Static) ? "STATIC" : "DYNAMIC";
  Fumbo::Graphic2D::DrawText(typeText, {(position.x - 20), (position.y - 30)},
                             {}, 10,
                             (bodyType == BodyType::Static) ? ORANGE : LIME);
}

void Object::UpdateVertices() {
  // This is called internally to update polygon/triangle transforms
  // The actual transformation is done in GetVe  DrawCircleLines(position.x,
  // position.y, 5, RED);
}

bool Object::IsCollidingWith(const Object *other) const {
  if (!other)
    return false;

  // Check collision regardless of collidable status
  // This allows detecting triggers and non-collidable objects
  auto contact = Collision::CheckCollision(this, other);
  return contact.hasCollision;
}

} // namespace Graphic2D
} // namespace Fumbo
