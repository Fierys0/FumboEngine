#include "button.hpp"

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

    // Move resource
    m_cacheTexture = other.m_cacheTexture;
    m_isDirty = other.m_isDirty;
    m_lastWidth = other.m_lastWidth;
    m_lastHeight = other.m_lastHeight;

    // Reset other
    other.m_cacheTexture = {0};
  }
  return *this;
}

Button::Button(Texture2D texture, Sound hoverSound, Sound clickSound)
    : texture(texture), hoverSound(hoverSound), clickSound(clickSound) {}

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

  // If a camera is provided (or stored), transform the mouse position to world space
  if (camera || m_camera) {
    Camera2D* cam = camera ? camera : m_camera;
    mouse = GetScreenToWorld2D(mouse, *cam);
  }
  
  bool isHovered = CheckCollisionPointRec(mouse, screenBounds);

  if (!m_interactable) {
    if (hovered) {
      hovered = false;
      m_isDirty = true;
    }
    return;
  }

  if (isHovered != hovered) {
    hovered = isHovered;
    m_isDirty = true; // State changed, redraw

    // Play hover sound once when mouse enters
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
  if (m_interactable != interactable) {
    m_interactable = interactable;
    m_isDirty = true;
  }
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

  // 2. Update Cache if Dirty
  if (m_isDirty && m_cacheTexture.id != 0) {
    BeginTextureMode(m_cacheTexture);
    ClearBackground(BLANK);

    // Draw visuals relative to (0,0) of the texture
    // Re-calculate local bounds

    // Draw Button Texture Scaled to fit
    Color tint = hovered ? m_hoveredColor : m_idleColor;
    if (!m_interactable)
      tint = m_disabledColor;
    DrawTexturePro(texture,
                   Rectangle{0, 0, (float)texture.width, (float)texture.height},
                   Rectangle{0, 0, (float)width, (float)height}, Vector2{0, 0},
                   0.0f, tint);

    // Draw text
    if (!text.empty()) {
      Vector2 scale = Fumbo::Utils::GetUIScale();
      float fontSize = baseFontSize * scale.y;

      float xPadding = 10.0f * scale.x; // Scale padding too
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
    if (m_camera)
      BeginMode2D(*m_camera);
    m_isDirty = false;
  }

  // 3. Draw Cache to Screen
  if (m_cacheTexture.id != 0) {
    Rectangle sourceRect = {0.0f, 0.0f, (float)m_cacheTexture.texture.width,
                        -(float)m_cacheTexture.texture.height};
    DrawTextureRec(m_cacheTexture.texture, sourceRect,
                   Vector2{screenBounds.x, screenBounds.y}, WHITE);
  }
} // Button::Draw()

void Button::SetBounds(Rectangle bounds) {
  this->uiBounds = bounds;
}

void Button::SetBounds(float x, float y, float w, float h) {
  this->uiBounds = {x, y, w, h};
}

bool Button::IsPressed() const {
  // Auto-update if not updated this frame
  double currentTime = GetTime();
  if (currentTime != m_lastUpdateTime) {
    const_cast<Button*>(this)->Update(m_camera);
  }
  return m_isPressed;
}

bool Button::IsReleased() const {
  double currentTime = GetTime();
  if (currentTime != m_lastUpdateTime) {
    const_cast<Button*>(this)->Update(m_camera);
  }
  return m_isReleased;
}

bool Button::IsHover() const {
  double currentTime = GetTime();
  if (currentTime != m_lastUpdateTime) {
    const_cast<Button*>(this)->Update(m_camera);
  }
  return hovered;
}
