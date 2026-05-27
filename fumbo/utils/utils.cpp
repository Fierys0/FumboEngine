#include "../../fumbo.hpp"
#include "raylib.h"
#include "raymath.h"
#include <algorithm> // For std::clamp
#include <cmath>

namespace Fumbo {
namespace Utils {
Vector2 GetUIScale() {
  float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
  float sc = std::fmin(sw / UI_WIDTH, sh / UI_HEIGHT);
  return {sc, sc};
}

Vector2 GetUIOffset() {
  float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
  float sc = std::fmin(sw / UI_WIDTH, sh / UI_HEIGHT);
  return {(sw - UI_WIDTH * sc) * 0.5f, (sh - UI_HEIGHT * sc) * 0.5f};
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
  Vector2 globalOffset = GetUIOffset();
  Rectangle dest = {drawX * uiScale.x + globalOffset.x, drawY * uiScale.y + globalOffset.y, drawW * uiScale.x,
                    drawH * uiScale.y};

  // Use raw raylib DrawTexturePro because coordinates are already scaled
  ::DrawTexturePro(texture, source, dest, {0, 0}, 0.0f, tint);
}

void DrawWorldSpriteAt(Rectangle worldRect, Texture2D texture,
                       Rectangle source, Color tint) {
  Vector2 uiScale = GetUIScale();
  Vector2 globalOffset = GetUIOffset();

  Rectangle dest = {
      worldRect.x * uiScale.x + globalOffset.x,
      worldRect.y * uiScale.y + globalOffset.y,
      worldRect.width * uiScale.x,
      worldRect.height * uiScale.y
  };

  ::DrawTexturePro(texture, source, dest, {0, 0}, 0.0f, tint);
}

void DrawWorldSpriteTiled(const Fumbo::Graphic2D::Object *object,
                          Texture2D texture, float tileSize,
                          TileMode tileMode, Color tint) {
  if (!object || texture.id == 0)
    return;

  Rectangle aabb = object->GetAABB();
  Vector2 uiScale = GetUIScale();
  Vector2 globalOffset = GetUIOffset();

  // tileSize = world units per full texture repeat on the tiled axis.
  // 0 means use the texture's native pixel width (or height).
  float tw = (tileSize > 0.0f) ? tileSize : (float)texture.width;
  float th = (tileSize > 0.0f) ? tileSize : (float)texture.height;

  float regionX = aabb.x;
  float regionY = aabb.y;
  float regionW = aabb.width;
  float regionH = aabb.height;

  bool tileX = (tileMode == TileMode::TILE_X || tileMode == TileMode::TILE_XY);
  bool tileY = (tileMode == TileMode::TILE_Y || tileMode == TileMode::TILE_XY);

  // One tile's drawn size on each axis
  float drawW = tileX ? tw : regionW;
  float drawH = tileY ? th : regionH;

  int tilesX = tileX ? (int)std::ceilf(regionW / tw) : 1;
  int tilesY = tileY ? (int)std::ceilf(regionH / th) : 1;

  // Full source rect of the texture
  Rectangle fullSrc = {0, 0, (float)texture.width, (float)texture.height};

  for (int ty = 0; ty < tilesY; ty++) {
    for (int tx = 0; tx < tilesX; tx++) {
      float worldX = regionX + tx * tw;
      float worldY = regionY + ty * th;

      // How much of this tile is still inside the region
      float clampW = std::fminf(drawW, regionX + regionW - worldX);
      float clampH = std::fminf(drawH, regionY + regionH - worldY);

      // Proportionally clip the SOURCE so we only show the visible portion
      // of the texture (never stretch a partial tile).
      Rectangle src = {
          fullSrc.x,
          fullSrc.y,
          fullSrc.width  * (clampW / drawW),
          fullSrc.height * (clampH / drawH)
      };

      Rectangle dest = {
          worldX * uiScale.x + globalOffset.x,
          worldY * uiScale.y + globalOffset.y,
          clampW * uiScale.x,
          clampH * uiScale.y
      };

      ::DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, tint);
    }
  }
}

} // namespace Utils
} // namespace Fumbo
