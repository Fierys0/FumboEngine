#include "../../fumbo.hpp"
#include "raylib.h"
#include "raymath.h"
#include <algorithm> // For std::clamp
#include <cmath>

namespace Fumbo {
namespace Utils {
Vector2 GetUIScale() {
  return {(float)GetScreenWidth() / UI_WIDTH,
          (float)GetScreenHeight() / UI_HEIGHT};
}

Vector2 CenterPosX(Vector2 objsize) {
  float screenWidth = UI_WIDTH; // Logic space center
  objsize.x = (screenWidth - objsize.x) * 0.5f;
  return objsize;
}

Vector2 CenterPosY(Vector2 objsize) {
  float screenHeight = UI_HEIGHT;
  objsize.y = (screenHeight - objsize.y) * 0.5f;
  return objsize;
}

Vector2 CenterPosXY(Vector2 objsize) {
  return {(UI_WIDTH - objsize.x) * 0.5f, (UI_HEIGHT - objsize.y) * 0.5f};
}

Rectangle UISpaceToScreen(Rectangle ui) {
  Vector2 s = GetUIScale();
  return {ui.x * s.x, ui.y * s.y, ui.width * s.x, ui.height * s.y};
}

void DrawPixelRuler(int spacing, Font font) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();

  // 1. Draw Horizontal Ruler (Top)
  for (int x = 0; x <= sw; x += 10) {
    int lineLength = (x % spacing == 0) ? 15 : 5;
    Color col = (x % spacing == 0) ? RED : DARKGRAY;

    DrawLine(x, 0, x, lineLength, col);

    if (x % spacing == 0) {
      // Convert int to C-string for Raylib
      std::string s = std::to_string(x);
      DrawTextEx(font, s.c_str(), {(float)x + 3, (float)lineLength}, 10, 1,
                 RED);
    }
  }

  // 2. Draw Vertical Ruler (Left)
  for (int y = 0; y <= sh; y += 10) {
    int lineLength = (y % spacing == 0) ? 15 : 5;
    Color col = (y % spacing == 0) ? RED : DARKGRAY;

    DrawLine(0, y, lineLength, y, col);

    if (y % spacing == 0) {
      std::string s = std::to_string(y);
      DrawTextEx(font, s.c_str(), {(float)lineLength + 3, (float)y + 3}, 10, 1,
                 RED);
    }
  }
}

bool MoveTowards(Vector2 &current, Vector2 target, float maxDistanceDelta) {
  float dx = target.x - current.x;
  float dy = target.y - current.y;
  float distsq = dx * dx + dy * dy;

  if (distsq == 0 || (maxDistanceDelta >= 0 &&
                      distsq <= maxDistanceDelta * maxDistanceDelta)) {
    current = target;
    return true;
  }

  float dist = std::sqrt(distsq);
  current.x += dx / dist * maxDistanceDelta;
  current.y += dy / dist * maxDistanceDelta;
  return false;
}

void Camera2DFollow(Camera2D *camera, Rectangle targetRect, float offsetx,
                    float offsety, float smoothness) {
  if (camera) {
    // Calculate center of rectangle
    Vector2 center = {targetRect.x + targetRect.width / 2.0f + offsetx,
                      targetRect.y + targetRect.height / 2.0f + offsety};
    Camera2DFollow(camera, center, 0, 0, smoothness);
  }
}

void Camera2DFollow(Camera2D *camera, Vector2 targetCenter, float offsetx,
                    float offsety, float smoothness) {
  if (camera) {
    Vector2 s = GetUIScale();
    Vector2 targetPos = {targetCenter.x * s.x, targetCenter.y * s.y};

    camera->offset = {(float)GetScreenWidth() / 2.0f + offsetx,
                      (float)GetScreenHeight() / 2.0f + offsety};

    // Zoom should be 1.0f because the coordinates are already scaled by
    // Graphic2D
    camera->zoom = 1.0f;

    if (smoothness > 0) {
      float dt = GetFrameTime();
      // Interpret smoothness as speed.
      float speed = (smoothness < 0.001f) ? 15.0f : smoothness;

      // Interpolate camera target towards real player position
      camera->target.x += (targetPos.x - camera->target.x) * speed * dt;
      camera->target.y += (targetPos.y - camera->target.y) * speed * dt;

    } else {
      // Instant snap
      camera->target = targetPos;
    }
  }
}

Texture2D ColorToTexture(Color color, Vector2 resolution) {
  if (resolution.x == 0 && resolution.y == 0) {
    resolution = {100, 100};
  }

  Image image = GenImageColor(resolution.x, resolution.y, color);
  Texture2D texture = LoadTextureFromImage(image);
  UnloadImage(image);
  return texture;
}

void DrawWorldSprite(const Fumbo::Graphic2D::Object *object, Texture2D texture,
                     Rectangle source, Vector2 offset, float scale,
                     Color tint) {
  if (!object)
    return;

  Rectangle aabb = object->GetAABB();
  Vector2 uiScale = GetUIScale();

  // Draw size is based on the SOURCE rectangle size scaled.
  // This allows sprites of different aspect ratios to render correctly.
  float drawW = fabsf(source.width) * scale;
  float drawH = source.height * scale;

  // Center the sprite on the object's center point
  float centerX = aabb.x + aabb.width * 0.5f;
  float centerY = aabb.y + aabb.height * 0.5f;

  float drawX = centerX - (drawW * 0.5f) + offset.x;
  float drawY = centerY - (drawH * 0.5f) + offset.y;

  // Scale to screen space (1280x720 virtual resolution)
  Rectangle dest = {drawX * uiScale.x, drawY * uiScale.y, drawW * uiScale.x,
                    drawH * uiScale.y};

  // Use raw raylib DrawTexturePro because coordinates are already scaled
  ::DrawTexturePro(texture, source, dest, {0, 0}, 0.0f, tint);
}

} // namespace Utils
} // namespace Fumbo
