#pragma once
#include <map>
#include <string>
#include <vector>

#include "raylib.h"
#include "raymath.h"

namespace Fumbo {
namespace UI {

// Text alignment options
enum class TextAlign {
  LEFT,
  CENTER,
  RIGHT
};

// Message box animation types
enum class BoxAnimation {
  NONE,
  FADE,
  SLIDE_UP,
  SLIDE_DOWN
};

// Message box style configuration
struct MessageBoxStyle {
  // Background
  Color backgroundColor = Color{0, 0, 0, 200};
  Texture2D backgroundTexture = {0};
  bool useTexture = false;
  bool useNinePatch = false;  // 9-slice scaling for texture
  
  // NinePatch settings (border sizes for 9-slice)
  int ninePatchLeft = 16;
  int ninePatchRight = 16;
  int ninePatchTop = 16;
  int ninePatchBottom = 16;
  
  // Border
  Color borderColor = WHITE;
  float borderThickness = 2.0f;
  float borderRounding = 0.0f;
  
  // Padding (space between box edge and text)
  float paddingTop = 20.0f;
  float paddingBottom = 20.0f;
  float paddingLeft = 20.0f;
  float paddingRight = 20.0f;
  
  // Shadow
  bool enableShadow = false;
  Vector2 shadowOffset = {4.0f, 4.0f};
  Color shadowColor = Color{0, 0, 0, 100};
  
  // Animation
  BoxAnimation animation = BoxAnimation::NONE;
  float animationDuration = 0.3f;
  float animationProgress = 1.0f;  // 0.0 to 1.0
};

// Text style configuration
struct TextStyle {
  TextAlign alignment = TextAlign::LEFT;
  float lineHeightMultiplier = 1.2f;
  
  // Text shadow
  bool enableShadow = false;
  Vector2 shadowOffset = {2.0f, 2.0f};
  Color shadowColor = Color{0, 0, 0, 150};
  
  // Text outline
  bool enableOutline = false;
  float outlineThickness = 1.0f;
  Color outlineColor = BLACK;
};

class VisualNovel {
public:
  VisualNovel(const std::string& text, float charsPerSecond);

  void Update(float deltaTime);
  void Draw(Font font, Vector2 startPos, float fontSize, float spacing, float maxX, float maxY,
            Color color);

  void Reset();
  void Clear();  // Unload scene (clear characters/text)
  void Skip();

  // New Features
  void SetSpeaker(const std::string& name, Color color = WHITE);
  void SetTypingSound(Sound sound);
  void SetText(const std::string& text);  // Helper to reset with new text
  bool IsComplete() const;  // Check if typing animation is complete


  // Character Sprites
  void AddCharacter(const std::string& name, Texture2D sprite, float scale = 1.0f);
  void SetCharacterPosition(const std::string& name, Vector2 pos);  // Instant Teleport
  void MoveCharacterPosition(const std::string& name, Vector2 targetPos,
                             float duration);  // Animated Move

  // Fading
  void SetCharacterAlpha(const std::string& name, float alpha);
  void FadeCharacter(const std::string& name, float targetAlpha, float duration);
  void FadeInCharacter(const std::string& name, float duration);
  void FadeOutCharacter(const std::string& name, float duration);

  void DrawSprites();  // Call before drawing text box

  // Message Box Customization
  void SetMessageBoxStyle(const MessageBoxStyle& style);
  void SetMessageBoxTexture(Texture2D texture, bool useNinePatch = false);
  void SetMessageBoxBounds(Rectangle bounds);  // Set position and size
  void EnableMessageBox(bool enable);
  
  // Text Customization
  void SetTextStyle(const TextStyle& style);
  void SetTextAlignment(TextAlign alignment);
  
  // Complete rendering (sprites + message box + text)
  void DrawComplete(Font font, float fontSize, float spacing, Color textColor = WHITE);

private:
  // Internal usage to hold process state
  struct Word {
    std::string text;
    Color color;
    bool isTag;
  };

  struct Character {
    Texture2D sprite;
    Vector2 position;
    float scale;
    Color tint;

    // Animation
    bool isMoving;
    Vector2 startPos;
    Vector2 targetPos;
    float moveTimer;
    float moveDuration;

    // Fading
    bool isFading;
    float fadeTimer;
    float fadeDuration;
    float startAlpha;
    float targetAlpha;
  };

  std::map<std::string, Character> m_characters;

  std::string m_rawText;
  std::vector<std::string> m_wrappedLines;

  // Speaker
  std::string m_speakerName;
  Color m_speakerColor = WHITE;

  // Audio
  Sound m_typeSound = {0};
  float m_soundTimer = 0.0f;

  // State
  int m_visibleChars;
  float m_timer;
  float m_charsPerSecond;
  
  // Message box and text styling
  MessageBoxStyle m_boxStyle;
  TextStyle m_textStyle;
  Rectangle m_boxBounds = {50, 500, 1180, 200};  // Default bounds
  bool m_enableMessageBox = false;

  void RecalculateWrapping(Font font, float fontSize, float spacing, float maxX);
  bool m_wrappingDone = false;
};
}  // namespace UI
}  // namespace Fumbo
