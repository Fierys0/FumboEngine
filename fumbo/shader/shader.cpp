#include "../../fumbo.hpp"
#include "raylib.h"

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

namespace Fumbo::Shaders {
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
  Vector2 normalizedPos = {position.x / (float)GetScreenWidth(),
                           position.y / (float)GetScreenHeight()};

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
} // namespace Fumbo::Shaders
