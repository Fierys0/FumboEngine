#include "../../fumbo.hpp"
#include <algorithm> // for clamp

namespace Fumbo {
namespace UI {

Slider::Slider(float min, float max, float initialValue)
    : m_min(min), m_max(max), m_value(initialValue) {
  if (m_value < m_min)
    m_value = m_min;
  if (m_value > m_max)
    m_value = m_max;
}

void Slider::SetValue(float value) {
  m_value = value;
  if (m_value < m_min)
    m_value = m_min;
  if (m_value > m_max)
    m_value = m_max;
}

float Slider::GetValue() const { return m_value; }

void Slider::SetStyle(const SliderConfig &config) { m_config = config; }

bool Slider::Update(Rectangle bounds) {
  Vector2 mousePos = GetMousePosition();
  bool changed = false;

  // Improve hit detection if knob is larger than bounds
  float effectiveKnobHeight =
      (m_config.knobHeight > 0) ? m_config.knobHeight : bounds.height;
  float extraY = (effectiveKnobHeight - bounds.height) / 2.0f;
  if (extraY < 0)
    extraY = 0;

  Rectangle hitRect = bounds;
  hitRect.y -= extraY;
  hitRect.height += extraY * 2;

  // Simple click & drag logic
  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    if (!m_dragging) {
      if (CheckCollisionPointRec(mousePos, hitRect)) {
        m_dragging = true;
      }
    }
  } else {
    m_dragging = false;
  }

  if (m_dragging) {
    float normalized = (mousePos.x - bounds.x) / bounds.width;
    if (normalized < 0.0f)
      normalized = 0.0f;
    if (normalized > 1.0f)
      normalized = 1.0f;

    float newValue = m_min + normalized * (m_max - m_min);

    if (newValue != m_value) {
      m_value = newValue;
      changed = true;
    }
  }

  return changed;
}

void Slider::Draw(Rectangle bounds) {
  // Calculation
  float range = m_max - m_min;
  float normalized = 0.0f;
  if (range > 0) {
    normalized = (m_value - m_min) / range;
  }

  // Track
  float trackHeight =
      (m_config.trackHeight > 0) ? m_config.trackHeight : bounds.height;
  float trackY = bounds.y + (bounds.height - trackHeight) / 2.0f;
  Rectangle trackRect = {bounds.x, trackY, bounds.width, trackHeight};

  if (m_config.trackTexture.id != 0) {
    DrawTexturePro(m_config.trackTexture,
                   {0, 0, (float)m_config.trackTexture.width,
                    (float)m_config.trackTexture.height},
                   trackRect, {0, 0}, 0.0f, WHITE);
  } else {
    DrawRectangleRec(trackRect, m_config.trackColor);
    if (m_config.outlineThickness > 0)
      DrawRectangleLinesEx(trackRect, m_config.outlineThickness,
                           m_config.outlineColor);
  }

  // Progress
  Rectangle progressRect = trackRect;
  progressRect.width = trackRect.width * normalized;

  if (m_config.progressTexture.id != 0) {
    float textureWidth = (float)m_config.progressTexture.width;
    float textureHeight = (float)m_config.progressTexture.height;
    Rectangle source = {0, 0, textureWidth * normalized, textureHeight};
    DrawTexturePro(m_config.progressTexture, source, progressRect, {0, 0}, 0.0f,
                   WHITE);
  } else {
    DrawRectangleRec(progressRect, m_config.progressColor);
  }

  // Knob
  float knobWidth = m_config.knobWidth;
  float knobHeight =
      (m_config.knobHeight > 0) ? m_config.knobHeight : bounds.height;

  // Center knob on current value
  float knobX = bounds.x + (bounds.width * normalized) - (knobWidth / 2);
  float knobY = bounds.y + (bounds.height - knobHeight) / 2.0f;

  Rectangle knobRect = {knobX, knobY, knobWidth, knobHeight};

  if (m_config.knobTexture.id != 0) {
    DrawTexturePro(m_config.knobTexture,
                   {0, 0, (float)m_config.knobTexture.width,
                    (float)m_config.knobTexture.height},
                   knobRect, {0, 0}, 0.0f, WHITE);
  } else {
    DrawRectangleRec(knobRect, m_config.knobColor);
    DrawRectangleLinesEx(knobRect, 1.0f, m_config.outlineColor);
  }
}

} // namespace UI
} // namespace Fumbo
