#include "../../fumbo.hpp"

namespace Fumbo {
  namespace UI {
    VisualNovel::VisualNovel(const std::string& text, float charsPerSecond)
        : m_rawText(text), m_visibleChars(0), m_timer(0.0f), m_charsPerSecond(charsPerSecond) {}

    void VisualNovel::SetSpeaker(const std::string& name, Color color) {
      m_speakerName = name;
      m_speakerColor = color;

      // Update Character Highlights

      bool speakerFound = (m_characters.find(name) != m_characters.end());

      if (speakerFound) {
        for (auto& pair : m_characters) {
          if (pair.first == name) {
            pair.second.tint = WHITE;
          } else {
            pair.second.tint = Color{100, 100, 100, 255};  // Dimmed
          }
        }
      }
      else if (!name.empty()) {
        for (auto& pair : m_characters) {
          pair.second.tint = Color{100, 100, 100, 255};
        }
      }
    }

    void VisualNovel::AddCharacter(const std::string& name, Texture2D sprite, float scale) {
      Character c;
      c.sprite = sprite;
      c.position = {0, 0};  // Default
      c.scale = scale;
      c.tint = Color{100, 100, 100, 255};  // Start dimmed
      c.isMoving = false;
      c.moveTimer = 0.0f;
      c.isFading = false;
      c.fadeTimer = 0.0f;
      m_characters[name] = c;
    }

    void VisualNovel::SetCharacterPosition(const std::string& name, Vector2 pos) {
      if (m_characters.find(name) != m_characters.end()) {
        m_characters[name].position = pos;
        m_characters[name].isMoving = false;  // Stop any anim
      }
    }

    void VisualNovel::MoveCharacterPosition(const std::string& name, Vector2 targetPos,
                                            float duration) {
      if (m_characters.find(name) != m_characters.end()) {
        Character& c = m_characters[name];
        c.isMoving = true;
        c.startPos = c.position;
        c.targetPos = targetPos;
        c.moveDuration = duration;
        c.moveTimer = 0.0f;
      }
    }

    void VisualNovel::SetCharacterAlpha(const std::string& name, float alpha) {
      if (m_characters.find(name) != m_characters.end()) {
        m_characters[name].tint.a = (unsigned char)std::clamp(alpha, 0.0f, 255.0f);
        m_characters[name].isFading = false;
      }
    }

    void VisualNovel::FadeCharacter(const std::string& name, float targetAlpha, float duration) {
      if (m_characters.find(name) != m_characters.end()) {
        Character& c = m_characters[name];
        c.isFading = true;
        c.startAlpha = (float)c.tint.a;
        c.targetAlpha = std::clamp(targetAlpha, 0.0f, 255.0f);
        c.fadeDuration = duration;
        c.fadeTimer = 0.0f;
      }
    }

    void VisualNovel::FadeInCharacter(const std::string& name, float duration) {
      FadeCharacter(name, 255.0f, duration);
    }

    void VisualNovel::FadeOutCharacter(const std::string& name, float duration) {
      FadeCharacter(name, 0.0f, duration);
    }

    void VisualNovel::DrawSprites() {
      for (const auto& pair : m_characters) {
        const Character& c = pair.second;
        Rectangle source = {0, 0, (float)c.sprite.width, (float)c.sprite.height};
        Rectangle dest = {c.position.x, c.position.y, c.sprite.width * c.scale, c.sprite.height * c.scale};
        Fumbo::Graphic2D::DrawTexturePro(c.sprite, source, dest, Vector2{0, 0}, 0.0f, c.tint);
      }
    }

    void VisualNovel::SetTypingSound(Sound sound) { m_typeSound = sound; }

    void VisualNovel::SetText(const std::string& text) {
      m_rawText = text;
      Reset();
      m_wrappedLines.clear();
      m_wrappingDone = false;
    }

    void VisualNovel::Reset() {
      m_timer = 0.0f;
      m_visibleChars = 0;
      // Don't clear lines here, just reset typing
    }

    void VisualNovel::Clear() {
      Reset();
      m_rawText.clear();
      m_wrappedLines.clear();
      m_wrappingDone = false;
      m_speakerName.clear();
      m_characters.clear();
    }

    void VisualNovel::Skip() {
      m_visibleChars = (int)m_rawText.length();
    }

    bool VisualNovel::IsComplete() const {
      return m_visibleChars >= (int)m_rawText.length();
    }


    void VisualNovel::Update(float deltaTime) {
      if (m_visibleChars < (int)m_rawText.length()) {
        m_timer += deltaTime;
        int previousVisible = m_visibleChars;
        m_visibleChars = static_cast<int>(m_timer * m_charsPerSecond);
        m_visibleChars = std::clamp(m_visibleChars, 0, (int)m_rawText.length());

        // Audio Feedback
        if (m_visibleChars > previousVisible && m_typeSound.stream.buffer != 0) {
          // Play every 2nd or 3rd char to avoid buzz
          if (m_visibleChars % 3 == 0) {
            PlaySound(m_typeSound);
          }
        }
      }

      // Update Character Animations
      for (auto& pair : m_characters) {
        Character& c = pair.second;
        if (c.isMoving) {
          c.moveTimer += deltaTime;
          float t = c.moveTimer / c.moveDuration;

          if (t >= 1.0f) {
            c.position = c.targetPos;
            c.isMoving = false;
          } else {
            // Safe Cubic Tune (EaseInOut)
            float smoothT = t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
            c.position = Vector2Lerp(c.startPos, c.targetPos, smoothT);
          }
        }

        if (c.isFading) {
          c.fadeTimer += deltaTime;
          float t = c.fadeTimer / c.fadeDuration;

          if (t >= 1.0f) {
            c.tint.a = (unsigned char)c.targetAlpha;
            c.isFading = false;
          } else {
            float current = c.startAlpha + (c.targetAlpha - c.startAlpha) * t;
            c.tint.a = (unsigned char)std::clamp(current, 0.0f, 255.0f);
          }
        }
      }
    }

    static std::vector<std::string> SplitWords(const std::string& s) {
      std::vector<std::string> words;
      std::string word;
      for (char c : s) {
        if (c == ' ' || c == '\n') {
          if (!word.empty()) words.push_back(word);
          if (c == '\n')
            words.push_back("\n");  // Preserve newlines
          else
            words.push_back(" ");  // Preserve spaces as words
          word.clear();
        } else {
          word += c;
        }
      }
      if (!word.empty()) words.push_back(word);
      return words;
    }

    void VisualNovel::RecalculateWrapping(Font font, float fontSize, float spacing, float maxX) {
      m_wrappedLines.clear();

      // Basic word wrapping
      // Note: Does not currently strip Tags like {RED} for width calculation, so tags might affect
      // wrapping slightly. For full rich text, we'd need to strip tags before measuring. Given the
      // complexity constraint, I will stick to Raw Text wrapping but ensure words don't break.

      auto words = SplitWords(m_rawText);
      std::string currentLine;
      float currentWidth = 0.0f;

      for (const auto& w : words) {
        if (w == "\n") {
          m_wrappedLines.push_back(currentLine);
          currentLine.clear();
          currentWidth = 0.0f;
          continue;
        }

        Vector2 size = MeasureTextEx(font, w.c_str(), fontSize, spacing);

        if (currentWidth + size.x > maxX && !currentLine.empty()) {
          m_wrappedLines.push_back(currentLine);
          currentLine = "";
          currentWidth = 0.0f;

          // If word is space, skip it at start of new line
          if (w == " ") continue;
        }

        currentLine += w;
        currentWidth += size.x;
      }
      if (!currentLine.empty()) m_wrappedLines.push_back(currentLine);

      m_wrappingDone = true;
    }

    void VisualNovel::Draw(Font font, Vector2 startPos, float fontSize, float spacing, float maxX,
                          float maxY, Color color) {
      // Recalculate wrapping if needed
      if (!m_wrappingDone) {
        RecalculateWrapping(font, fontSize, spacing, maxX);
      }

      // Draw Speaker Name
      if (!m_speakerName.empty()) {
        Vector2 namePos = {startPos.x, startPos.y - fontSize - 10};
        DrawTextEx(font, m_speakerName.c_str(), namePos, fontSize * 1.1f, spacing, m_speakerColor);
      }

      // Draw Text Lines
      Vector2 pen = startPos;
      int charsProcessed = 0;

      for (const std::string& line : m_wrappedLines) {
        if (charsProcessed >= m_visibleChars) break;

        std::string lineToDraw = line;

        // Check if we need to cut the line
        int charsNeeded = m_visibleChars - charsProcessed;
        if (charsNeeded < (int)line.length()) {
          lineToDraw = line.substr(0, charsNeeded);
        }

        DrawTextEx(font, lineToDraw.c_str(), pen, fontSize, spacing, color);

        pen.y += fontSize + spacing;      // Next line
        charsProcessed += line.length();  // Count logic is approx if tags existed
      }
    }

    // ===== New Customization Methods =====

    void VisualNovel::SetMessageBoxStyle(const MessageBoxStyle& style) {
      m_boxStyle = style;
    }

    void VisualNovel::SetMessageBoxTexture(Texture2D texture, bool useNinePatch) {
      m_boxStyle.backgroundTexture = texture;
      m_boxStyle.useTexture = (texture.id != 0);
      m_boxStyle.useNinePatch = useNinePatch;
    }

    void VisualNovel::SetMessageBoxBounds(Rectangle bounds) {
      m_boxBounds = bounds;
    }

    void VisualNovel::EnableMessageBox(bool enable) {
      m_enableMessageBox = enable;
    }

    void VisualNovel::SetTextStyle(const TextStyle& style) {
      m_textStyle = style;
    }

    void VisualNovel::SetTextAlignment(TextAlign alignment) {
      m_textStyle.alignment = alignment;
    }

    // Helper function to draw NinePatch (9-slice) texture
    static void DrawNinePatch(Texture2D texture, Rectangle dest, int left, int right, int top, int bottom, Color tint) {
      int srcWidth = texture.width;
      int srcHeight = texture.height;
      int centerWidth = srcWidth - left - right;
      int centerHeight = srcHeight - top - bottom;
      float destCenterWidth = dest.width - left - right;
      float destCenterHeight = dest.height - top - bottom;
      
      // Top-left
      DrawTexturePro(texture, Rectangle{0, 0, (float)left, (float)top},
                     Rectangle{dest.x, dest.y, (float)left, (float)top}, Vector2{0, 0}, 0.0f, tint);
      // Top
      DrawTexturePro(texture, Rectangle{(float)left, 0, (float)centerWidth, (float)top},
                     Rectangle{dest.x + left, dest.y, destCenterWidth, (float)top}, Vector2{0, 0}, 0.0f, tint);
      // Top-right
      DrawTexturePro(texture, Rectangle{(float)(srcWidth - right), 0, (float)right, (float)top},
                     Rectangle{dest.x + dest.width - right, dest.y, (float)right, (float)top}, Vector2{0, 0}, 0.0f, tint);
      // Left
      DrawTexturePro(texture, Rectangle{0, (float)top, (float)left, (float)centerHeight},
                     Rectangle{dest.x, dest.y + top, (float)left, destCenterHeight}, Vector2{0, 0}, 0.0f, tint);
      // Center
      DrawTexturePro(texture, Rectangle{(float)left, (float)top, (float)centerWidth, (float)centerHeight},
                     Rectangle{dest.x + left, dest.y + top, destCenterWidth, destCenterHeight}, Vector2{0, 0}, 0.0f, tint);
      // Right
      DrawTexturePro(texture, Rectangle{(float)(srcWidth - right), (float)top, (float)right, (float)centerHeight},
                     Rectangle{dest.x + dest.width - right, dest.y + top, (float)right, destCenterHeight}, Vector2{0, 0}, 0.0f, tint);
      // Bottom-left
      DrawTexturePro(texture, Rectangle{0, (float)(srcHeight - bottom), (float)left, (float)bottom},
                     Rectangle{dest.x, dest.y + dest.height - bottom, (float)left, (float)bottom}, Vector2{0, 0}, 0.0f, tint);
      // Bottom
      DrawTexturePro(texture, Rectangle{(float)left, (float)(srcHeight - bottom), (float)centerWidth, (float)bottom},
                     Rectangle{dest.x + left, dest.y + dest.height - bottom, destCenterWidth, (float)bottom}, Vector2{0, 0}, 0.0f, tint);
      // Bottom-right
      DrawTexturePro(texture, Rectangle{(float)(srcWidth - right), (float)(srcHeight - bottom), (float)right, (float)bottom},
                     Rectangle{dest.x + dest.width - right, dest.y + dest.height - bottom, (float)right, (float)bottom}, Vector2{0, 0}, 0.0f, tint);
    }

    void VisualNovel::DrawComplete(Font font, float fontSize, float spacing, Color textColor) {
      DrawSprites();
      
      if (!m_enableMessageBox) {
        float maxX = m_boxBounds.width - m_boxStyle.paddingLeft - m_boxStyle.paddingRight;
        float maxY = m_boxBounds.height - m_boxStyle.paddingTop - m_boxStyle.paddingBottom;
        Vector2 textPos = {m_boxBounds.x + m_boxStyle.paddingLeft, m_boxBounds.y + m_boxStyle.paddingTop};
        Draw(font, textPos, fontSize, spacing, maxX, maxY, textColor);
        return;
      }
      
      Rectangle animatedBounds = m_boxBounds;
      Color bgColor = m_boxStyle.backgroundColor;
      
      if (m_boxStyle.animation == BoxAnimation::FADE) {
        bgColor.a = (unsigned char)(bgColor.a * m_boxStyle.animationProgress);
      } else if (m_boxStyle.animation == BoxAnimation::SLIDE_UP) {
        animatedBounds.y = m_boxBounds.y + (m_boxBounds.height * (1.0f - m_boxStyle.animationProgress));
      } else if (m_boxStyle.animation == BoxAnimation::SLIDE_DOWN) {
        animatedBounds.y = m_boxBounds.y - (m_boxBounds.height * (1.0f - m_boxStyle.animationProgress));
      }
      
      if (m_boxStyle.enableShadow) {
        Rectangle shadowRect = {animatedBounds.x + m_boxStyle.shadowOffset.x, animatedBounds.y + m_boxStyle.shadowOffset.y,
                                animatedBounds.width, animatedBounds.height};
        if (m_boxStyle.borderRounding > 0) {
          Fumbo::Graphic2D::DrawRectangleRounded(shadowRect, m_boxStyle.borderRounding / animatedBounds.height, 16, m_boxStyle.shadowColor);
        } else {
          Fumbo::Graphic2D::DrawRectangleRec(shadowRect, m_boxStyle.shadowColor);
        }
      }
      
      if (m_boxStyle.useTexture && m_boxStyle.backgroundTexture.id != 0) {
        if (m_boxStyle.useNinePatch) {
          DrawNinePatch(m_boxStyle.backgroundTexture, animatedBounds, m_boxStyle.ninePatchLeft, m_boxStyle.ninePatchRight,
                       m_boxStyle.ninePatchTop, m_boxStyle.ninePatchBottom, WHITE);
        } else {
          Fumbo::Graphic2D::DrawTexturePro(m_boxStyle.backgroundTexture,
                        Rectangle{0, 0, (float)m_boxStyle.backgroundTexture.width, (float)m_boxStyle.backgroundTexture.height},
                        animatedBounds, Vector2{0, 0}, 0.0f, WHITE);
        }
      } else {
        if (m_boxStyle.borderRounding > 0) {
          Fumbo::Graphic2D::DrawRectangleRounded(animatedBounds, m_boxStyle.borderRounding / animatedBounds.height, 16, bgColor);
        } else {
          Fumbo::Graphic2D::DrawRectangleRec(animatedBounds, bgColor);
        }
      }
      
      if (m_boxStyle.borderThickness > 0) {
        if (m_boxStyle.borderRounding > 0) {
          Fumbo::Graphic2D::DrawRectangleRoundedLines(animatedBounds, m_boxStyle.borderRounding / animatedBounds.height, 16, m_boxStyle.borderColor);
        } else {
          Fumbo::Graphic2D::DrawRectangleLinesEx(animatedBounds, m_boxStyle.borderThickness, m_boxStyle.borderColor);
        }
      }
      
      if (!m_speakerName.empty()) {
        Vector2 namePos = {animatedBounds.x + m_boxStyle.paddingLeft, animatedBounds.y - fontSize * 1.1f - 10};
        if (m_textStyle.enableShadow) {
          Vector2 shadowPos = {namePos.x + m_textStyle.shadowOffset.x, namePos.y + m_textStyle.shadowOffset.y};
          Fumbo::Graphic2D::DrawText(m_speakerName, shadowPos, font, fontSize * 1.1f, m_textStyle.shadowColor);
        }
        Fumbo::Graphic2D::DrawText(m_speakerName, namePos, font, fontSize * 1.1f, m_speakerColor);
      }
      
      float textAreaWidth = animatedBounds.width - m_boxStyle.paddingLeft - m_boxStyle.paddingRight;
      float textAreaHeight = animatedBounds.height - m_boxStyle.paddingTop - m_boxStyle.paddingBottom;
      Vector2 textStartPos = {animatedBounds.x + m_boxStyle.paddingLeft, animatedBounds.y + m_boxStyle.paddingTop};
      
      if (!m_wrappingDone) {
        RecalculateWrapping(font, fontSize, spacing, textAreaWidth);
      }
      
      Vector2 pen = textStartPos;
      int charsProcessed = 0;
      float lineHeight = fontSize * m_textStyle.lineHeightMultiplier;
      
      for (const std::string& line : m_wrappedLines) {
        if (charsProcessed >= m_visibleChars) break;
        
        std::string lineToDraw = line;
        int charsNeeded = m_visibleChars - charsProcessed;
        if (charsNeeded < (int)line.length()) {
          lineToDraw = line.substr(0, charsNeeded);
        }
        
        float xOffset = 0.0f;
        if (m_textStyle.alignment == TextAlign::CENTER) {
          Vector2 textSize = MeasureTextEx(font, lineToDraw.c_str(), fontSize, spacing);
          xOffset = (textAreaWidth - textSize.x) / 2.0f;
        } else if (m_textStyle.alignment == TextAlign::RIGHT) {
          Vector2 textSize = MeasureTextEx(font, lineToDraw.c_str(), fontSize, spacing);
          xOffset = textAreaWidth - textSize.x;
        }
        
        Vector2 linePos = {pen.x + xOffset, pen.y};
        
        if (m_textStyle.enableOutline) {
          for (int ox = -1; ox <= 1; ox++) {
            for (int oy = -1; oy <= 1; oy++) {
              if (ox == 0 && oy == 0) continue;
              Vector2 outlinePos = {linePos.x + ox * m_textStyle.outlineThickness, linePos.y + oy * m_textStyle.outlineThickness};
              Fumbo::Graphic2D::DrawText(lineToDraw, outlinePos, font, fontSize, m_textStyle.outlineColor);
            }
          }
        }
        
        if (m_textStyle.enableShadow) {
          Vector2 shadowPos = {linePos.x + m_textStyle.shadowOffset.x, linePos.y + m_textStyle.shadowOffset.y};
          Fumbo::Graphic2D::DrawText(lineToDraw, shadowPos, font, fontSize, m_textStyle.shadowColor);
        }
        
        Fumbo::Graphic2D::DrawText(lineToDraw, linePos, font, fontSize, textColor);
        
        pen.y += lineHeight;
        charsProcessed += line.length();
      }
    }

  }  // namespace UI
}  // namespace Fumbo
