#include "../../fumbo.hpp"
#include "raymath.h"
#include <algorithm>
#include <cmath>

namespace Fumbo {
namespace Graphic2D {

Physics::Physics()
    : gravity({0, 980.0f}), // Default gravity (pixels/s^2)
      fixedTimeStep(1.0f / 60.0f), accumulator(0.0f), iterations(4),
      debugDraw(false) {}

// Object Management

void Physics::AddObject(Object *object) {
  if (object && std::find(objects.begin(), objects.end(), object) == objects.end()) {
    objects.push_back(object);
  }
}

void Physics::RemoveObject(Object *object) {
  auto it = std::find(objects.begin(), objects.end(), object);
  if (it != objects.end()) {
    objects.erase(it);
  }
}

void Physics::Clear() { objects.clear(); }

// Physics Simulation

void Physics::Update(float deltaTime) {
  // Fixed timestep accumulator
  accumulator += deltaTime;

  while (accumulator >= fixedTimeStep) {
    Step(fixedTimeStep);
    accumulator -= fixedTimeStep;
  }
}

void Physics::Step(float deltaTime) {
  // Apply gravity
  ApplyGravity(deltaTime);

  // Update all objects
  for (auto *object : objects) {
    object->Update(deltaTime);
  }

  // Resolve collisions (multiple iterations for stability)
  for (int i = 0; i < iterations; i++) {
    ResolveCollisions();
  }
}

void Physics::ApplyGravity(float deltaTime) {
  for (auto *object : objects) {
    if (object->GetBodyType() == BodyType::Dynamic) {
      Vector2 gravityForce =
          Vector2Scale(gravity, object->GetMass() * object->GetGravityScale());
      object->ApplyForce(gravityForce);
    }
  }
}

void Physics::ResolveCollisions() {
  for (size_t i = 0; i < objects.size(); i++) {
    for (size_t j = i + 1; j < objects.size(); j++) {
      Object *objectA = objects[i];
      Object *objectB = objects[j];

      // Skip if either object is non-collidable
      if (!objectA->IsCollidable() || !objectB->IsCollidable())
        continue;

      // Skip if both are static
      if (objectA->GetBodyType() == BodyType::Static &&
          objectB->GetBodyType() == BodyType::Static) {
        continue;
      }

      // Broadphase: Quick AABB overlap check before expensive collision detection
      Rectangle aabbA = objectA->GetAABB();
      Rectangle aabbB = objectB->GetAABB();
      
      if (!CheckCollisionRecs(aabbA, aabbB)) {
        continue; // No AABB overlap, skip expensive narrow-phase check
      }

      // Check collision
      CollisionContact contact = Collision::CheckCollision(objectA, objectB);

      if (contact.hasCollision) {
        // If either is a trigger, don't resolve physics (just notify)
        if (objectA->IsTrigger() || objectB->IsTrigger()) {
          // TODO: Add trigger callback system
          continue;
        }

        // Resolve collision
        ResolveCollision(objectA, objectB, contact);
      }
    }
  }
}

void Physics::ResolveCollision(Object *objectA, Object *objectB,
                                const CollisionContact &contact) {
  // Calculate relative velocity
  Vector2 relativeVelocity = Vector2Subtract(objectB->GetVelocity(), objectA->GetVelocity());
  float velAlongNormal = Vector2DotProduct(relativeVelocity, contact.normal);

  // Don't resolve if objects are separating
  if (velAlongNormal > 0)
    return;

  // Calculate restitution (bounciness)
  float restitution = fminf(objectA->GetRestitution(), objectB->GetRestitution());

  // Calculate impulse scalar
  float impulseScalar = -(1.0f + restitution) * velAlongNormal;

  bool aIsStatic = (objectA->GetBodyType() == BodyType::Static);
  bool bIsStatic = (objectB->GetBodyType() == BodyType::Static);

  float invMassA = aIsStatic ? 0.0f : 1.0f / objectA->GetMass();
  float invMassB = bIsStatic ? 0.0f : 1.0f / objectB->GetMass();
  float invMassSum = invMassA + invMassB;

  if (invMassSum > 0.0001f) {
    impulseScalar /= invMassSum;
  }

  // Apply impulse
  Vector2 impulse = Vector2Scale(contact.normal, impulseScalar);

  if (!aIsStatic) {
    objectA->ApplyImpulse(Vector2Scale(impulse, -1.0f));
  }
  if (!bIsStatic) {
    objectB->ApplyImpulse(impulse);
  }

  // Positional correction to prevent sinking
  const float percent = 0.4f; // Penetration percentage to correct
  const float slop = 0.01f;   // Penetration allowance
  float correctionAmount =
      fmaxf(contact.penetration - slop, 0.0f) / invMassSum * percent;
  Vector2 correction = Vector2Scale(contact.normal, correctionAmount);

  if (!aIsStatic) {
    Vector2 newPos =
        Vector2Subtract(objectA->GetPosition(), Vector2Scale(correction, invMassA));
    objectA->SetPosition(newPos);
  }
  if (!bIsStatic) {
    Vector2 newPos =
        Vector2Add(objectB->GetPosition(), Vector2Scale(correction, invMassB));
    objectB->SetPosition(newPos);
  }

  // Apply friction
  Vector2 tangent =
      Vector2Subtract(relativeVelocity, Vector2Scale(contact.normal, velAlongNormal));
  float tangentLength = Vector2Length(tangent);

  if (tangentLength > 0.0001f) {
    tangent = Vector2Scale(tangent, 1.0f / tangentLength);

    float friction = sqrtf(objectA->GetFriction() * objectB->GetFriction());
    float frictionImpulse = -Vector2DotProduct(relativeVelocity, tangent);
    frictionImpulse /= invMassSum;
    frictionImpulse *= friction;

    Vector2 frictionVector = Vector2Scale(tangent, frictionImpulse);

    if (!aIsStatic) {
      objectA->ApplyImpulse(Vector2Scale(frictionVector, -1.0f));
    }
    if (!bIsStatic) {
      objectB->ApplyImpulse(Vector2Scale(frictionVector, 1.0f));
    }
  }
}

// Raycasting

RaycastHit Physics::Raycast(Vector2 origin, Vector2 direction,
                            float maxDistance) {
  RaycastHit result;
  result.hit = false;
  result.distance = maxDistance;
  result.object = nullptr;

  Vector2 directionNormalized = Vector2Normalize(direction);
  Vector2 rayEnd = Vector2Add(origin, Vector2Scale(directionNormalized, maxDistance));

  for (auto *object : objects) {
    // Simple raycast using line-shape intersection
    ShapeType type = object->GetShapeType();

    if (type == ShapeType::Circle) {
      // Ray-circle intersection
      Vector2 toCircle = Vector2Subtract(object->GetPosition(), origin);
      float projection = Vector2DotProduct(toCircle, directionNormalized);

      if (projection < 0)
        continue; // Behind ray

      Vector2 closest = Vector2Add(origin, Vector2Scale(directionNormalized, projection));
      float distToCenter = Vector2Distance(closest, object->GetPosition());

      if (distToCenter <= object->GetRadius()) {
        float offset = sqrtf(object->GetRadius() * object->GetRadius() -
                             distToCenter * distToCenter);
        float hitDist = projection - offset;

        if (hitDist >= 0 && hitDist < result.distance) {
          result.hit = true;
          result.distance = hitDist;
          result.object = object;
          result.point = Vector2Add(origin, Vector2Scale(directionNormalized, hitDist));
          result.normal = Vector2Normalize(
              Vector2Subtract(result.point, object->GetPosition()));
        }
      }
    } else if (type == ShapeType::Rectangle || type == ShapeType::Polygon ||
               type == ShapeType::Triangle) {
      // Ray-polygon intersection
      auto vertices = object->GetVertices();
      for (size_t i = 0; i < vertices.size(); i++) {
        Vector2 point1 = vertices[i];
        Vector2 point2 = vertices[(i + 1) % vertices.size()];

        Vector2 intersection;
        if (Collision::LineIntersection(origin, rayEnd, point1, point2,
                                        &intersection)) {
          float dist = Vector2Distance(origin, intersection);
          if (dist < result.distance) {
            result.hit = true;
            result.distance = dist;
            result.object = object;
            result.point = intersection;

            // Calculate normal from edge
            Vector2 edge = Vector2Subtract(point2, point1);
            result.normal = Vector2Normalize({-edge.y, edge.x});
          }
        }
      }
    }
  }

  return result;
}

std::vector<RaycastHit> Physics::RaycastAll(Vector2 origin, Vector2 direction,
                                            float maxDistance) {
  std::vector<RaycastHit> hits;

  Vector2 directionNormalized = Vector2Normalize(direction);
  Vector2 rayEnd = Vector2Add(origin, Vector2Scale(directionNormalized, maxDistance));

  for (auto *object : objects) {
    RaycastHit tempResult;
    tempResult.hit = false;
    tempResult.distance = maxDistance;

    ShapeType type = object->GetShapeType();

    if (type == ShapeType::Circle) {
      Vector2 toCircle = Vector2Subtract(object->GetPosition(), origin);
      float projection = Vector2DotProduct(toCircle, directionNormalized);

      if (projection >= 0) {
        Vector2 closest = Vector2Add(origin, Vector2Scale(directionNormalized, projection));
        float distToCenter = Vector2Distance(closest, object->GetPosition());

        if (distToCenter <= object->GetRadius()) {
          float offset = sqrtf(object->GetRadius() * object->GetRadius() -
                               distToCenter * distToCenter);
          float hitDist = projection - offset;

          if (hitDist >= 0) {
            tempResult.hit = true;
            tempResult.distance = hitDist;
            tempResult.object = object;
            tempResult.point = Vector2Add(origin, Vector2Scale(directionNormalized, hitDist));
            tempResult.normal = Vector2Normalize(
                Vector2Subtract(tempResult.point, object->GetPosition()));
          }
        }
      }
    } else if (type == ShapeType::Rectangle || type == ShapeType::Polygon ||
               type == ShapeType::Triangle) {
      auto vertices = object->GetVertices();
      for (size_t i = 0; i < vertices.size(); i++) {
        Vector2 point1 = vertices[i];
        Vector2 point2 = vertices[(i + 1) % vertices.size()];

        Vector2 intersection;
        if (Collision::LineIntersection(origin, rayEnd, point1, point2,
                                        &intersection)) {
          float dist = Vector2Distance(origin, intersection);
          if (!tempResult.hit || dist < tempResult.distance) {
            tempResult.hit = true;
            tempResult.distance = dist;
            tempResult.object = object;
            tempResult.point = intersection;

            Vector2 edge = Vector2Subtract(point2, point1);
            tempResult.normal = Vector2Normalize({-edge.y, edge.x});
          }
        }
      }
    }

    if (tempResult.hit) {
      hits.push_back(tempResult);
    }
  }

  // Sort by distance
  std::sort(hits.begin(), hits.end(),
            [](const RaycastHit &hitA, const RaycastHit &hitB) {
              return hitA.distance < hitB.distance;
            });

  return hits;
}

// Debug Rendering

void Physics::DrawDebug() const {
  if (!debugDraw)
    return;

  for (const auto *object : objects) {
    object->DrawDebug();
  }

  // Draw gravity direction
  Vector2 gravDir = Vector2Normalize(gravity);
  Vector2 gravStart = {50, 50};
  Vector2 gravEnd = Vector2Add(gravStart, Vector2Scale(gravDir, 30));
  ::DrawLineEx(gravStart, gravEnd, 3.0f, MAGENTA);
  ::DrawText("GRAVITY", 60, 45, 10, MAGENTA);
}

} // namespace Graphic2D
} // namespace Fumbo
