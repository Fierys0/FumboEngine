#include "shadermanager.hpp"

namespace Fumbo {

const char* blurShaderCode = R"(
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
// uniform vec4 colDiffuse; // Not typically sent by default for 2D batching in this manner

out vec4 finalColor;

uniform float renderWidth;
uniform float renderHeight;
uniform float radius;

void main()
{
    if (radius <= 0.0) 
    {
        finalColor = texture(texture0, fragTexCoord) * fragColor;
        return;
    }

    vec4 sum = vec4(0.0);
    int blurSize = int(radius);
    int count = 0;

    for (int x = -blurSize; x <= blurSize; ++x)
    {
        for (int y = -blurSize; y <= blurSize; ++y)
        {
            sum += texture(texture0, fragTexCoord + vec2(x, y) / vec2(renderWidth, renderHeight));
            count++;
        }
    }

    finalColor = (sum / float(count)) * fragColor;
}
)";

void ShaderManager::Init(int width, int height) {
  blurShader = LoadShaderFromMemory(nullptr, blurShaderCode);
  locRenderWidth = GetShaderLocation(blurShader, "renderWidth");
  locRenderHeight = GetShaderLocation(blurShader, "renderHeight");
  locRadius = GetShaderLocation(blurShader, "radius");

  blurTarget = LoadRenderTexture(width, height);
}

void ShaderManager::Cleanup() {
  UnloadShader(blurShader);
  UnloadRenderTexture(blurTarget);
}

void ShaderManager::BeginBlurMode(float radius) {
  float w = (float)GetScreenWidth();
  float h = (float)GetScreenHeight();

  SetShaderValue(blurShader, locRenderWidth, &w, SHADER_UNIFORM_FLOAT);
  SetShaderValue(blurShader, locRenderHeight, &h, SHADER_UNIFORM_FLOAT);
  SetShaderValue(blurShader, locRadius, &radius, SHADER_UNIFORM_FLOAT);

  BeginShaderMode(blurShader);
}

void ShaderManager::EndBlurMode() { EndShaderMode(); }

void ShaderManager::BeginBlurPass() {
  BeginTextureMode(blurTarget);
  ClearBackground(BLANK);  // Ensure transparency
  blurPassActive = true;
}

void ShaderManager::EndBlurPass(float radius, Vector2 pos) {
  if (!blurPassActive) return;

  EndTextureMode();
  blurPassActive = false;

  // Draw the result with blur
  BeginBlurMode(radius);
  // DrawTextureRec renders a part of texture defined by rec.
  // We need to flip Y because OpenGL coordinates for RenderTextures are inverted relative to Raylib
  // drawing.
  Rectangle src = {0, 0, (float)blurTarget.texture.width, -(float)blurTarget.texture.height};
  DrawTextureRec(blurTarget.texture, src, pos, WHITE);
  EndBlurMode();
}

void ShaderManager::DrawBlur(Texture2D texture, Vector2 pos, float radius) {
  BeginBlurMode(radius);
  DrawTextureV(texture, pos, WHITE);
  EndBlurMode();
}

}  // namespace Fumbo
