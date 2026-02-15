#pragma once
#include "fumbo/2d/graphics.hpp"
#include "fumbo/2d/physics.hpp"
#include "fumbo/audio/audiomanager.hpp"
#include "fumbo/shader/shadermanager.hpp"
#include "fumbo/ui/button.hpp"
#include "fumbo/ui/slider.hpp"
#include "fumbo/ui/visualn.hpp"
#include "fumbo/utils/assets.hpp"
#include "fumbo/utils/fades.hpp"
#include "fumbo/utils/utils.hpp"
#include "fumbo/video/videoplayer.hpp"
#include "igamestate.hpp"
#include "raylib.h"
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Fumbo {

class Engine {
public:
  static Engine &Instance() {
    static Engine instance;
    return instance;
  }
  void Init(int width = 1280, int height = 720,
            const std::string &title = "Fumbo Engine", double targetFPS = 0);
  // Frame Limiter
  void LimitFPS(double fps);
  void SetVSync(bool enabled);

  // Debug / UI
  void DrawFPS(int x, int y);
  void DrawFPS(int x, int y, Font font, int fontSize, Color color);

  void Run(std::shared_ptr<IGameState> initialState);
  void Cleanup();
  void Quit();

  // State Management
  void ChangeState(std::shared_ptr<IGameState> newState);

  // Engine Services
  FadeManager &GetFader() { return ShaderManager::Instance().GetFader(); }

  ShaderManager &GetShaderManager() { return ShaderManager::Instance(); }

  Audio::AudioManager &GetAudioManager() {
    return Audio::AudioManager::Instance();
  }

  Graphic2D::Physics &GetPhysics() { return Graphic2D::Physics::Instance(); }

  // Global Accessors (Wrappers around Raylib or Utils)
  int GetWidth() const;
  int GetHeight() const;

  // Clean Layer Management
  void InvalidateCleanLayer() { m_cleanIsInvalid = true; }
  bool IsCleanLayerInvalid() const { return m_cleanIsInvalid; }

  // Global Overlay System
  using OverlayCallback = std::function<void()>;
  void AddGlobalOverlay(OverlayCallback cb);

private:
  Engine() = default;
  ~Engine() = default;
  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;

  bool running = true;
  std::shared_ptr<IGameState> currentState;
  std::shared_ptr<IGameState> nextState;
  bool stateChangePending = false;
  std::vector<OverlayCallback> m_globalOverlays;

  // Clean Layer Caching
  RenderTexture2D m_cleanTexture = {0};
  bool m_cleanIsInvalid = true;

  void Update();
  void Draw();
  static void SetFumboIcon();
};

// Convenience helper to avoid breaking changes in user code
inline Engine &Instance() { return Engine::Instance(); }

} // namespace Fumbo
