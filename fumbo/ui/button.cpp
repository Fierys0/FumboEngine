#include "../../fumbo.hpp"
#include "raylib.h"

using namespace Fumbo::UI;

Button::~Button() {
  if (m_cacheTexture.id != 0)
    UnloadRenderTexture(m_cacheTexture);
}

Button::Button(Button &&other) noexcept { *this = std::move(other); }

Button &Button::operator=(Button &&other) noexcept {
  if (this != &other) {
    // Cleanup current
    if (m_cacheTexture.id != 0)
      UnloadRenderTexture(m_cacheTexture);

    // Move data
    uiBounds = other.uiBounds;
    texture = other.texture;
    text = std::move(other.text);
    font = other.font;
    baseFontSize = other.baseFontSize;
    hoverSound = other.hoverSound;
    clickSound = other.clickSound;
    horizontalAlign = other.horizontalAlign;
    verticalAlign = other.verticalAlign;
    hovered = other.hovered;

    // Move missing fields
    m_segments = other.m_segments;
    m_roundness = other.m_roundness;
    m_thickness = other.m_thickness;
    m_textColor = other.m_textColor;
    buttonColor = other.buttonColor;
    m_hoveredColor = other.m_hoveredColor;
    m_idleColor = other.m_idleColor;
    m_disabledColor = other.m_disabledColor;
    m_textOffsetX = other.m_textOffsetX;
    m_textOffsetY = other.m_textOffsetY;
    m_interactable = other.m_interactable;
    m_camera = other.m_camera;
    m_worldSpace = other.m_worldSpace;
    m_isPressed = other.m_isPressed;
    m_isReleased = other.m_isReleased;
    m_lastUpdateTime = other.m_lastUpdateTime;

    // Move resource
    m_cacheTexture = other.m_cacheTexture;
    m_isDirty = other.m_isDirty;
    m_lastWidth = other.m_lastWidth;
    m_lastHeight = other.m_lastHeight;

    // Reset other
    other.m_cacheTexture = {0};
    other.m_lastUpdateTime = -1.0;
  }
  return *this;
}

Button::Button(Rectangle uiBounds, Color buttonColor, Texture2D texture)
    : uiBounds(uiBounds), buttonColor(buttonColor), texture(texture) {}

void Button::Update(Camera2D *camera) {
  // Reset per-frame states
  m_isPressed = false;
  m_isReleased = false;

  Rectangle screenBounds;
  if (m_worldSpace) {
    screenBounds = uiBounds; // No scaling for world space
  } else {
    screenBounds = Fumbo::Utils::UISpaceToScreen(uiBounds);
  }

  this->Position = Vector2{screenBounds.x, screenBounds.y};
  Vector2 mouse = GetMousePosition();

  // If a camera is provided (or stored), transform the mouse position to world
  // space
  if (camera || m_camera) {
    Camera2D *cam = camera ? camera : m_camera;
    mouse = GetScreenToWorld2D(mouse, *cam);
  }

  bool isHovered = CheckCollisionPointRec(mouse, screenBounds);

  if (!m_interactable) {
    hovered = false; // Tint applied at blit time, no FBO redraw
    return;
  }

  if (isHovered != hovered) {
    hovered = isHovered;
    // Hover tint is applied at blit time — no FBO redraw needed
    if (isHovered) {
      PlaySound(hoverSound);
    }
  }

  if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    m_isPressed = true;
    PlaySound(clickSound);
  }

  if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    m_isReleased = true;
  }

  m_lastUpdateTime = GetTime(); // Mark as updated this frame
}

void Button::SetInteractable(bool interactable) {
  // Disabled tint applied at blit time, FBO redraw not needed
  m_interactable = interactable;
}

void Button::SetWorldSpace(bool worldSpace) {
  if (m_worldSpace != worldSpace) {
    m_worldSpace = worldSpace;
    m_isDirty = true;
  }
}

void Button::AddText(const std::string &text, Font font, int fontSize,
                     Color color) {
  this->text = text;
  this->font = font;
  this->baseFontSize = fontSize;
  m_textColor = color;
  m_isDirty = true;
}

void Button::AddText(const std::string &text, Color color) {
  AddText(text, this->font, this->baseFontSize, color);
  m_isDirty = true;
}

void Button::AlignText(ButtonAlign x, ButtonAlign y) {
  this->horizontalAlign = x;
  this->verticalAlign = y;
  m_isDirty = true;
}

void Button::HoveredColor(Color color) { m_hoveredColor = color; }

void Button::IdleColor(Color color) { m_idleColor = color; }

void Button::DisabledColor(Color color) { m_disabledColor = color; }

void Button::TextOffsetX(float offset) { m_textOffsetX = offset; }

void Button::TextOffsetY(float offset) { m_textOffsetY = offset; }

void Button::TextOffsetXY(float offsetX, float offsetY) {
  m_textOffsetX = offsetX;
  m_textOffsetY = offsetY;
}

void Button::Draw() {
  Rectangle screenBounds;
  if (m_worldSpace) {
    screenBounds = uiBounds;
  } else {
    screenBounds = Fumbo::Utils::UISpaceToScreen(uiBounds);
  }

  int width = (int)screenBounds.width;
  int height = (int)screenBounds.height;

  // 1. Check if Cache needs resize or init
  if (m_cacheTexture.id == 0 || width != m_lastWidth ||
      height != m_lastHeight) {
    if (m_cacheTexture.id != 0)
      UnloadRenderTexture(m_cacheTexture);
    if (width > 0 && height > 0) {
      m_cacheTexture = LoadRenderTexture(width, height);
      m_lastWidth = width;
      m_lastHeight = height;
      m_isDirty = true;
    }
  }

  // 2. Update Cache if Dirty (structure/content only — hover tint handled at
  // blit)
  if (m_isDirty && m_cacheTexture.id != 0) {
    BeginTextureMode(m_cacheTexture);
    ClearBackground(BLANK);

    if (IsTextureValid(texture)) {
      // Bake base appearance at WHITE — hover/disabled tint applied cheaply at
      // blit time
      DrawTexturePro(
          texture, Rectangle{0, 0, (float)texture.width, (float)texture.height},
          Rectangle{0, 0, (float)width, (float)height}, Vector2{0, 0}, 0.0f,
          WHITE);
    } else {
      DrawRectangleRounded(Rectangle{1, 1, (float)width - 2, (float)height - 2},
                           m_roundness, m_segments, buttonColor);
    }

    // Draw text
    if (!text.empty()) {
      Vector2 scale = Fumbo::Utils::GetUIScale();
      float fontSize = baseFontSize * scale.y;

      float xPadding = 10.0f * scale.x;
      float yPadding = 10.0f * scale.y;

      Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, 1);
      Vector2 textPos = {0, 0};

      // Horizontal
      switch (horizontalAlign) {
      case ButtonAlign::LEFT:
        textPos.x = xPadding;
        break;
      case ButtonAlign::MIDDLE:
        textPos.x = (width - textSize.x) / 2;
        break;
      case ButtonAlign::RIGHT:
        textPos.x = width - textSize.x - xPadding;
        break;
      default:
        textPos.x = (width - textSize.x) / 2;
        break;
      }

      textPos.x += m_textOffsetX;

      // Vertical
      switch (verticalAlign) {
      case ButtonAlign::TOP:
        textPos.y = yPadding;
        break;
      case ButtonAlign::MIDDLE:
        textPos.y = (height - textSize.y) / 2;
        break;
      case ButtonAlign::BOTTOM:
        textPos.y = height - textSize.y - yPadding;
        break;
      default:
        textPos.x = (width - textSize.x) / 2;
        break;
      }

      textPos.y += m_textOffsetY;

      DrawTextEx(font, text.c_str(), textPos, fontSize, 1, m_textColor);
    }

    EndTextureMode();
    m_isDirty = false;
  }

  // 3. Blit Cache to Screen — apply hover/disabled tint here, zero FBO cost
  if (m_cacheTexture.id != 0) {
    if (m_worldSpace && m_camera)
      BeginMode2D(*m_camera);

    Rectangle sourceRect = {0.0f, 0.0f, (float)m_cacheTexture.texture.width,
                            -(float)m_cacheTexture.texture.height};
    Color blitTint = m_interactable ? (hovered ? m_hoveredColor : m_idleColor)
                                    : m_disabledColor;
    DrawTextureRec(m_cacheTexture.texture, sourceRect,
                   Vector2{screenBounds.x, screenBounds.y}, blitTint);

    if (m_worldSpace && m_camera)
      EndMode2D();
  }
} // Button::Draw()

void Button::SetBounds(Rectangle bounds) {
  // Guard: skip if nothing changed to avoid triggering unnecessary size checks
  if (uiBounds.x == bounds.x && uiBounds.y == bounds.y &&
      uiBounds.width == bounds.width && uiBounds.height == bounds.height)
    return;
  uiBounds = bounds;
}

void Button::SetBounds(float x, float y, float w, float h) {
  SetBounds({x, y, w, h});
}

bool Button::IsPressed() const {
  // Auto-update if not updated this frame
  double currentTime = GetTime();
  if (std::abs(currentTime - m_lastUpdateTime) > 0.001) {
    const_cast<Button *>(this)->Update(m_camera);
  }
  return m_isPressed;
}

bool Button::IsReleased() const {
  double currentTime = GetTime();
  if (std::abs(currentTime - m_lastUpdateTime) > 0.001) {
    const_cast<Button *>(this)->Update(m_camera);
  }
  return m_isReleased;
}

bool Button::IsHover() const {
  double currentTime = GetTime();
  if (std::abs(currentTime - m_lastUpdateTime) > 0.001) {
    const_cast<Button *>(this)->Update(m_camera);
  }
  return hovered;
}

void Button::SetButtonSound(Sound clickSound, Sound hoverSound) {
  this->clickSound = clickSound;
  this->hoverSound = hoverSound;
}

void Button::SetButtonColor(Color color) {
  this->buttonColor = color;
  m_isDirty = true;
}

void Button::SetTexture(Texture2D texture) {
  m_isDirty = true;
  this->texture = texture;
}

void Button::ApplyStyle(const ButtonStyle &style) {
  this->font = style.font;
  this->baseFontSize = style.fontsize;
  this->m_idleColor = style.idleColor;
  this->m_hoveredColor = style.hoveredColor;
  this->m_disabledColor = style.DisabledColor;
  this->texture = style.texture;
  this->m_roundness = style.roundness;
  this->m_segments = (int)style.segments;
  this->m_isDirty = true;
}
