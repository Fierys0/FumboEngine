#include "fumbo.hpp"
#include "fumbo_icon.h"
#include "raylib.h"
#include <fstream>
#include <sstream>

void Fumbo::Engine::Init(int width, int height, const std::string &title,
                         double targetFPS) {
  InitWindow(width, height, title.c_str());
  InitAudioDevice();
  SetFumboIcon();
  LimitFPS(targetFPS);
  SetExitKey(0);

  // Initialize Shaders
  GetShaderManager().Init(width, height);
}

void Fumbo::Engine::LimitFPS(double fps) { SetTargetFPS(fps); }

void Fumbo::Engine::SetVSync(bool enabled) {
  if (enabled) {
    SetWindowState(FLAG_VSYNC_HINT);
  } else {
    ClearWindowState(FLAG_VSYNC_HINT);
  }
}

void Fumbo::Engine::Cleanup() {
  if (currentState) {
    currentState->Cleanup();
  }

  GetShaderManager().Cleanup();
  GetAudioManager().Cleanup();

  if (m_cleanTexture.id != 0)
    UnloadRenderTexture(m_cleanTexture);

  CloseAudioDevice();
  CloseWindow();
}

void Fumbo::Engine::Quit() { running = false; }

void Fumbo::Engine::ChangeState(std::shared_ptr<IGameState> newState) {
  nextState = newState;
  stateChangePending = true;
}

void Fumbo::Engine::Run(std::shared_ptr<IGameState> initialState) {
  ChangeState(initialState);

  while (!WindowShouldClose() && running) {
    Update();
    Draw();
  }

  Cleanup();
}

void Fumbo::Engine::Update() {
  if (stateChangePending) {
    if (currentState) {
      currentState->Cleanup();
    }
    currentState = nextState;
    currentState->Init();
    InvalidateCleanLayer(); // Force redraw of clean layer for new state
    stateChangePending = false;
  }

  // Engine Systems Update
  GetAudioManager().Update();
  // Physics is updated per-state/area, not globally

  if (currentState) {
    currentState->Update();
  }
}

void Fumbo::Engine::AddGlobalOverlay(OverlayCallback cb) {
  m_globalOverlays.push_back(cb);
}

void Fumbo::Engine::Draw() {
  // Handle Window Resize
  if (IsWindowResized()) {
    if (m_cleanTexture.id != 0)
      UnloadRenderTexture(m_cleanTexture);
    m_cleanTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    InvalidateCleanLayer();
  }

  // Ensure texture exists (first run)
  if (m_cleanTexture.id == 0) {
    m_cleanTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    InvalidateCleanLayer();
  }

  if (currentState) {
    // 1. Update Clean Layer Cache if needed
    if (m_cleanIsInvalid) {
      BeginTextureMode(m_cleanTexture);
      ClearBackground(RAYWHITE);

      currentState->DrawClean();
      EndTextureMode();
      m_cleanIsInvalid = false;
    }

    // 2. Draw Final Frame
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw Cached Clean Layer
    // Note: RenderTextures are flipped vertically in OpenGL
    Rectangle srcRec = {0.0f, 0.0f, (float)m_cleanTexture.texture.width,
                        -(float)m_cleanTexture.texture.height};
    DrawTextureRec(m_cleanTexture.texture, srcRec, Vector2{0, 0}, WHITE);

    // Draw Dirty Layer (Dynamic)
    currentState->DrawDirty();

    // Draw Global Overlays
    for (const auto &overlay : m_globalOverlays) {
      overlay();
    }

    EndDrawing();
  } else {
    // Fallback if no state
    BeginDrawing();
    ClearBackground(BLUE);
    EndDrawing();
  }
}

int Fumbo::Engine::GetWidth() const { return GetScreenWidth(); }

int Fumbo::Engine::GetHeight() const { return GetScreenHeight(); }

void Fumbo::Engine::DrawFPS(int x, int y) {
  int fps = GetFPS();
  Color color = LIME;
  if (fps < 15)
    color = RED;
  else if (fps < 30)
    color = ORANGE;

  DrawFPS(x, y, GetFontDefault(), 20, color);
}

void Fumbo::Engine::DrawFPS(int x, int y, Font font, int fontSize,
                            Color color) {
  std::string text = std::to_string(GetFPS()) + " FPS";
  // Or use TextFormat if strict Raylib style desired, but std::string is safer

  Graphic2D::DrawText(text, Vector2{(float)x, (float)y}, font, fontSize, color);
}

void Fumbo::Engine::SetFumboIcon() {
  Image icon =
      LoadImageFromMemory(".ico", engine_fumbo_ico, engine_fumbo_ico_len);
  SetWindowIcon(icon);
  UnloadImage(icon);
}
