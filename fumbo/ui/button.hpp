#pragma once
#include <string>

#include "../utils/utils.hpp"
#include "raylib.h"

enum class ButtonAlign { LEFT, MIDDLE, RIGHT, TOP, BOTTOM };
namespace Fumbo {
namespace UI {
class Button {
public:
  ~Button();
  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;
  Button(Button &&other) noexcept;
  Button &operator=(Button &&other) noexcept;

  Button() = default;

  Button(Texture2D texture, Sound hoverSound = {}, Sound clickSound = {});

  void Draw();

  void AddText(const std::string &text, Font font, int fontSize,
               Color color = BLACK);

  void AlignText(ButtonAlign vertical = ButtonAlign::MIDDLE,
                 ButtonAlign horizontal = ButtonAlign::MIDDLE);

  void HoveredColor(Color color);
  void IdleColor(Color color);
  void DisabledColor(Color color);
  void SetPosition(Vector2 Position);
  void SetBounds(Rectangle bounds);
  void SetBounds(float x, float y, float w, float h);

  void TextOffsetX(float offset);
  void TextOffsetY(float offset);
  void TextOffsetXY(float offsetX, float offsetY);

  Vector2 Position;
  void SetInteractable(bool interactable);
  void SetWorldSpace(bool worldSpace);
  bool IsPressed() const;
  bool IsReleased() const;
  bool IsHover() const;

private:
  void Update(Camera2D *camera = nullptr);
  Rectangle uiBounds{};
  Texture2D texture{};
  std::string text;
  Font font{};
  int baseFontSize{};
  Color m_textColor = BLACK;

  Sound hoverSound{};
  Sound clickSound{};

  ButtonAlign horizontalAlign = ButtonAlign::MIDDLE;
  ButtonAlign verticalAlign = ButtonAlign::MIDDLE;

  bool hovered = false;
  Color m_hoveredColor = WHITE;
  Color m_idleColor = {200, 200, 200, 255};
  Color m_disabledColor = Fade(m_idleColor, 0.5f);
  float m_textOffsetX = 0;
  float m_textOffsetY = 0;

  // Caching
  RenderTexture2D m_cacheTexture = {0};
  bool m_isDirty = true;
  bool m_interactable = true;
  int m_lastWidth = 0;
  int m_lastHeight = 0;
  Camera2D *m_camera = nullptr;
  bool m_worldSpace = false;
  
  // State tracking for auto-update
  bool m_isPressed = false;
  bool m_isReleased = false;
  mutable double m_lastUpdateTime = -1.0;
};
} // namespace UI
} // namespace Fumbo
