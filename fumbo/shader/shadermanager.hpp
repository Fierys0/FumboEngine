#pragma once
#include "../utils/fades.hpp"
#include "raylib.h"

namespace Fumbo {

  class ShaderManager {
  public:
    static ShaderManager& Instance() {
      static ShaderManager instance;
      return instance;
    }

    void Init(int width, int height);
    void Cleanup();

    void BeginBlurMode(float radius);
    void EndBlurMode();

    // Blur Pass: Compositing multiple objects
    void BeginBlurPass();
    void EndBlurPass(float radius, Vector2 pos = {0, 0});

    // Draw content with blur shader applied
    void DrawBlur(Texture2D texture, Vector2 pos, float radius);

    FadeManager& GetFader() { return fader; }

  private:
    ShaderManager() = default;
    ~ShaderManager() = default;

    Shader blurShader = {0};
    int locRenderWidth = -1;
    int locRenderHeight = -1;
    int locRadius = -1;

    RenderTexture2D blurTarget = {0};
    bool blurPassActive = false;

    FadeManager fader;
  };

}  // namespace Fumbo
