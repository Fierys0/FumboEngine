#pragma once
#include "raylib.h"

namespace Fumbo {
namespace UI {

struct SliderConfig {
  // Colors
  Color trackColor = LIGHTGRAY;
  Color progressColor = SKYBLUE;
  Color knobColor = DARKGRAY;
  Color outlineColor = DARKGRAY;
  
  // Dimensions
  float knobWidth = 20.0f;
  float knobHeight = 0.0f; // 0 = match bounds height
  float trackHeight = 0.0f; // 0 = match bounds height
  float outlineThickness = 1.0f;

  // Textures (Optional, if .id != 0 they are used)
  Texture2D trackTexture = {0};
  Texture2D progressTexture = {0};
  Texture2D knobTexture = {0};
};

class Slider {
public:
  Slider() = default;
  Slider(float min, float max, float initialValue);

  // Returns true if value changed
  bool Update(Rectangle bounds);
  void Draw(Rectangle bounds);

  void SetValue(float value);
  float GetValue() const;

  void SetStyle(const SliderConfig& config);

private:
  float m_min = 0.0f;
  float m_max = 1.0f;
  float m_value = 0.5f;

  bool m_dragging = false;

  SliderConfig m_config;
};

} // namespace UI
} // namespace Fumbo
