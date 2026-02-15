#pragma once
#include "../../fumbo.hpp"
#include "../../template/topdown.hpp"

class TopDown : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  Texture2D bgTex;
  Camera2D camera = {{0, 0}, {0, 0}, 0.0f, 1.0f};

  // Top-down character
  Fumbo::Graphic2D::Object *player = nullptr;
  Fumbo::TopDown::TopDownController *controller = nullptr;

  // Walls
  std::vector<Fumbo::Graphic2D::Object *> walls;

  bool showDebug = true;
};
