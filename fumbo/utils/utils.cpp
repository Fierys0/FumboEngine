#include "../../fumbo.hpp"
#include "raylib.h"
#include "raymath.h"
#include <algorithm> // For std::clamp
#include <cmath>

const char *solidShader = R"(
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform vec4 solidColor; // The global color you want to apply

void main()
{
    // Get the alpha value from the texture
    float alpha = texture(texture0, fragTexCoord).a;
    
    // Replace RGB with solidColor, keep original texture Alpha
    finalColor = vec4(solidColor.rgb, solidColor.a * alpha);
}
  )";

const char *radialBlurShader =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec2 center;\n"        // Center of blur (0.0 to 1.0)
    "uniform float blurStrength;\n" // How strong the blur is
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 color = vec4(0.0);\n"
    "    vec2 dir = fragTexCoord - center;\n" // Direction from center
    "    float dist = length(dir);\n"
    "    dir = normalize(dir);\n"
    "    \n"
    "    // Sample along the ray from center\n"
    "    int samples = 32;\n" // More samples = smoother but slower
    "    for (int i = 0; i < samples; i++)\n"
    "    {\n"
    "        float offset = (float(i) / float(samples)) * blurStrength;\n"
    "        vec2 samplePos = fragTexCoord - dir * offset;\n"
    "        color += texture(texture0, samplePos);\n"
    "    }\n"
    "    \n"
    "    finalColor = color / float(samples);\n"
    "}\n";

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

void MakeSolidColor(Texture2D texture, Color color) {
  Color *pixels = LoadImageColors(LoadImageFromTexture(texture));

  for (int i = 0; i < texture.width * texture.height; i++) {
    if (pixels[i].a > 0) { // If pixel is not transparent
      pixels[i] = color;
    }
  }
  UpdateTexture(texture, pixels);
  UnloadImageColors(pixels);
}

Shader MakeSolidColorShader(Color color) {
  Shader solid = LoadShaderFromMemory(0, solidShader);
  int solidLoc = GetShaderLocation(solid, "solidColor");
  float colorfloat[4] = {(float)color.r / 255, (float)color.g / 255,
                         (float)color.b / 255, (float)color.a / 255};
  SetShaderValue(solid, solidLoc, colorfloat, SHADER_UNIFORM_VEC4);
  return solid;
}

Texture2D DrawMask(RenderTexture2D screen, RenderTexture2D &renderTarget,
                   Shader &maskShader, Color canvasColor, Color maskColor,
                   bool forceRecreate) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // Check if we need to recreate resources
  bool needsRecreate = forceRecreate || renderTarget.id == 0 ||
                       renderTarget.texture.width != screenWidth ||
                       renderTarget.texture.height != screenHeight;

  if (needsRecreate) {
    // Cleanup old resources
    if (renderTarget.id != 0) {
      UnloadRenderTexture(renderTarget);
    }
    if (maskShader.id != 0) {
      UnloadShader(maskShader);
    }

    // Create new resources
    renderTarget = LoadRenderTexture(screenWidth, screenHeight);
    maskShader = MakeSolidColorShader(maskColor);
  }

  // Render to texture
  BeginTextureMode(renderTarget);
  ClearBackground(canvasColor);
  BeginShaderMode(maskShader);
  DrawTexturePro(screen.texture,
                 (Rectangle){0, 0, (float)screenWidth, -(float)screenHeight},
                 (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                 (Vector2){0, 0}, 0.0f, WHITE);

  EndShaderMode();
  EndTextureMode();

  // Return the texture (no flip needed, will handle in shader/drawing)
  return renderTarget.texture;
}

Texture2D ApplyRadialBlur(Texture2D source, RenderTexture2D &canvas,
                          Shader &shader, float blurValue, Vector2 position,
                          bool forceRecreate) {
  // Check if we need to recreate resources
  bool needsRecreate = forceRecreate || canvas.id == 0 || shader.id == 0 ||
                       canvas.texture.width != source.width ||
                       canvas.texture.height != source.height;

  // Static variables to cache shader uniform locations
  static int centerLoc = -1;
  static int blurStrengthLoc = -1;
  static unsigned int cachedShaderId = 0;

  if (needsRecreate) {
    // Cleanup old resources
    if (canvas.id != 0) {
      UnloadRenderTexture(canvas);
    }
    if (shader.id != 0) {
      UnloadShader(shader);
    }

    // Create new resources
    canvas = LoadRenderTexture(source.width, source.height);
    shader = LoadShaderFromMemory(NULL, radialBlurShader);
    
    // Cache shader uniform locations for new shader
    centerLoc = GetShaderLocation(shader, "center");
    blurStrengthLoc = GetShaderLocation(shader, "blurStrength");
    cachedShaderId = shader.id;
  } else if (shader.id != cachedShaderId) {
    // Shader was recreated externally, update cached locations
    centerLoc = GetShaderLocation(shader, "center");
    blurStrengthLoc = GetShaderLocation(shader, "blurStrength");
    cachedShaderId = shader.id;
  }

  // Apply blur
  BeginTextureMode(canvas);
  ClearBackground(BLANK);
  BeginShaderMode(shader);

  // Normalize position to 0-1 range based on actual screen dimensions
  Vector2 normalizedPos = {
      position.x / (float)GetScreenWidth(),
      position.y / (float)GetScreenHeight()
  };
  
  // Set shader parameters using cached locations
  SetShaderValue(shader, centerLoc, &normalizedPos, SHADER_UNIFORM_VEC2);
  SetShaderValue(shader, blurStrengthLoc, &blurValue, SHADER_UNIFORM_FLOAT);

  DrawTexturePro(source,
                 (Rectangle){0, 0, (float)source.width, (float)source.height},
                 (Rectangle){0, 0, (float)source.width, (float)source.height},
                 (Vector2){0, 0}, 0.0f, WHITE);
  EndShaderMode();
  EndTextureMode();

  // Return the texture directly (no image flip needed)
  return canvas.texture;
}
} // namespace Utils
} // namespace Fumbo
