#pragma once
#include "../../fumbo.hpp"
#include "../../template/platformer.hpp"

class PlatformerExample : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;
  void DrawScene();

private:
  Texture2D bgTex{};
  Camera2D camera;

  // Physics objects
  Fumbo::Graphic2D::Object *ground = nullptr;
  Fumbo::Graphic2D::Object *platform1 = nullptr;
  Fumbo::Graphic2D::Object *platform2 = nullptr;

  Fumbo::Graphic2D::Object *player = nullptr;
  std::vector<Fumbo::Graphic2D::Object *> crates;

  // Collectibles & Enemies
  std::vector<Fumbo::Graphic2D::Object *> coins;
  Fumbo::Graphic2D::Object *enemy = nullptr;
  int score = 0;
  bool gameOver = false;

  // The magic controller - handles all movement!
  Fumbo::Platformer::PlatformerController *controller = nullptr;

  bool showDebug = true;
  int coinCounter = 0;
  Vector2 oldEnemyPos;
  float timerDelta;

  // Render mode toggle
  enum class RenderMode { NORMAL, MASK, GODRAYS };
  RenderMode renderMode = RenderMode::NORMAL;

  // Cached resources for god rays effect (performance optimization)
  RenderTexture2D maskRenderTarget = {0};
  Shader maskShader = {0};
  RenderTexture2D blurRenderTarget = {0};
  Shader blurShader = {0};
  RenderTexture2D currentScreen;

  // Track screen size for resize detection
  int lastScreenWidth = 0;
  int lastScreenHeight = 0;
};
