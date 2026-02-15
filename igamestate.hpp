#pragma once

// Abstract base class for all game states
class IGameState {
public:
  virtual ~IGameState() = default;

  virtual void Init() = 0;
  virtual void Cleanup() = 0;

  virtual void Pause() {}
  virtual void Resume() {}

  virtual void Update() = 0;
  virtual void DrawClean() = 0;
  virtual void DrawDirty() = 0;
};
