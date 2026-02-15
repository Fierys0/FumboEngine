#pragma once
#include "fades.hpp"
#include "raylib.h"
#include <functional>
#include <string>

namespace Fumbo {
namespace Utils {
constexpr float UI_WIDTH = 1280.0f;
constexpr float UI_HEIGHT = 720.0f;

// UI scale
Vector2 GetUIScale();

// Coordinate helpers
Vector2 CenterPosX(Vector2 objsize);
Vector2 CenterPosY(Vector2 objsize);
Vector2 CenterPosXY(Vector2 objsize);
Rectangle UISpaceToScreen(Rectangle ui);
void DrawPixelRuler(int spacing = 50, Font font = {});
bool MoveTowards(Vector2 &current, Vector2 target, float maxDistanceDelta);
void Camera2DFollow(Camera2D *camera, Rectangle targetRect, float offsetx = 0,
                    float offsety = 0, float smoothness = 0.0f);
void Camera2DFollow(Camera2D *camera, Vector2 targetCenter, float offsetx = 0,
                    float offsety = 0, float smoothness = 0.0f);
void MakeSolidColor(Texture2D texture, Color color);
Shader MakeSolidColorShader(Color color);

// Cached versions for performance - reuse resources across frames
Texture2D DrawMask(RenderTexture2D screen, RenderTexture2D &renderTarget,
                   Shader &maskShader, Color canvasColor = WHITE,
                   Color maskColor = BLACK, bool forceRecreate = false);
Texture2D ApplyRadialBlur(Texture2D source, RenderTexture2D &canvas,
                          Shader &shader, float blurValue = 0.1f,
                          Vector2 position = {UI_WIDTH / 2, UI_HEIGHT / 2},
                          bool forceRecreate = false);

// Internal / private-like helpers
namespace Internal {
void GameSettings();
}
} // namespace Utils
} // namespace Fumbo
