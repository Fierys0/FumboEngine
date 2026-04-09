#include "../../fumbo.hpp"
#include "raymath.h"
#include <algorithm>

namespace Fumbo {
namespace UI {

Textbox::Textbox(Rectangle bounds, Font font, int fontSize)
    : m_bounds(bounds), m_font(font), m_fontSize(fontSize) {}

void Textbox::SetStyle(const TextboxConfig &config) { m_config = config; }

void Textbox::SetBounds(Rectangle bounds) { m_bounds = bounds; }

void Textbox::SetText(const std::string &text) {
  m_text = text;
  m_cursorPos = m_text.length();
}

void Textbox::SetFont(Font font, int fontSize) {
  m_font = font;
  m_fontSize = fontSize;
}

void Textbox::SetMultiline(bool multiline) { m_multiline = multiline; }

void Textbox::SetMaxLines(int maxLines) { m_maxLines = maxLines; }

void Textbox::SetMaxLength(int maxLength) { m_maxLength = maxLength; }

void Textbox::SetInteractable(bool interactable) { m_interactable = interactable; }

void Textbox::SetFocused(bool focused) { m_focused = focused; }

void Textbox::SetWorldSpace(bool worldSpace) { m_worldSpace = worldSpace; }

// Direct Style Setters
void Textbox::SetBackgroundTexture(Texture2D texture) { m_config.backgroundTexture = texture; }
void Textbox::SetBackgroundColor(Color color) { m_config.backgroundColor = color; }
void Textbox::SetOutlineColor(Color color, Color focusedColor) {
  m_config.outlineColor = color;
  m_config.focusedOutlineColor = focusedColor;
}
void Textbox::SetTextColor(Color color) { m_config.textColor = color; }
void Textbox::SetPadding(Vector2 padding) { m_config.padding = padding; }

void Textbox::SetSelectionColor(Color color) { m_config.selectionColor = color; }

std::string Textbox::GetText() const { return m_text; }

bool Textbox::IsFocused() const { return m_focused; }

Rectangle Textbox::GetBounds() const { return m_bounds; }

void Textbox::Update() {
  if (!m_interactable) return;

  Vector2 mousePos = GetMousePosition();

  Rectangle screenBounds;
  if (m_worldSpace) {
    screenBounds = m_bounds;
  } else {
    screenBounds = Fumbo::Utils::UISpaceToScreen(m_bounds);
  }

  // Calculate necessary visual values for mouse checking
  Vector2 scale = {1.0f, 1.0f};
  if (!m_worldSpace) scale = Fumbo::Utils::GetUIScale();

  Rectangle textArea = {
      screenBounds.x + (m_config.padding.x * scale.x),
      screenBounds.y + (m_config.padding.y * scale.y),
      screenBounds.width - (m_config.padding.x * scale.x * 2),
      screenBounds.height - (m_config.padding.y * scale.y * 2)
  };
  Vector2 textPos = {textArea.x - m_scrollOffset, textArea.y};
  float scaledFontSize = m_fontSize * scale.y;
  if (!m_multiline) {
      // rough alignment calculation for single line Y
      textPos.y = textArea.y + (textArea.height - scaledFontSize) / 2.0f;
  }

  // Handle Focus & Mouse Selection
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    if (CheckCollisionPointRec(mousePos, screenBounds)) {
      m_focused = true;
      m_cursorVisible = true;
      m_cursorTimer = 0.0f;
      m_isDragging = true;
      
      // Calculate cursor pos based on click
      m_cursorPos = GetCursorIndexFromMouse(mousePos, textPos, scale.y);
      m_selectStart = m_cursorPos;
      m_selectLength = 0;
    } else {
      m_focused = false;
      m_isDragging = false;
      m_selectStart = -1;
      m_selectLength = 0;
    }
  }

  if (m_isDragging) {
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
          int dragPos = GetCursorIndexFromMouse(mousePos, textPos, scale.y);
          m_cursorPos = dragPos;
          m_selectLength = dragPos - m_selectStart;
          m_cursorVisible = true;
      } else {
          m_isDragging = false;
      }
  }

  if (m_focused) {
    // Cursor blinking timer
    m_cursorTimer += GetFrameTime();
    if (m_cursorTimer >= 0.5f) {
      m_cursorVisible = !m_cursorVisible;
      m_cursorTimer = 0.0f;
    }

    HandleInput();
  } else {
    m_cursorVisible = false;
  }
}

void Textbox::DeleteSelection() {
    if (m_selectStart >= 0 && m_selectLength != 0) {
        int start = m_selectLength > 0 ? m_selectStart : m_selectStart + m_selectLength;
        int len = abs(m_selectLength);
        m_text.erase(start, len);
        m_cursorPos = start;
        m_selectStart = -1;
        m_selectLength = 0;
    }
}

void Textbox::InsertTextAtCursor(const std::string& inserted) {
    DeleteSelection();
    
    std::string toInsert = inserted;

    // Remove newlines if not multiline
    if (!m_multiline) {
        toInsert.erase(std::remove(toInsert.begin(), toInsert.end(), '\n'), toInsert.end());
        toInsert.erase(std::remove(toInsert.begin(), toInsert.end(), '\r'), toInsert.end());
    }

    // Check Max Length Limit
    if (m_maxLength > 0) {
        int allowedChars = m_maxLength - m_text.length();
        if (allowedChars <= 0) return;
        
        if (toInsert.length() > allowedChars) {
            toInsert = toInsert.substr(0, allowedChars);
            // Ensure we don't truncate a multi-byte UTF-8 character in half
            while (!toInsert.empty() && (toInsert.back() & 0xC0) == 0x80) {
                toInsert.pop_back();
            }
            if (!toInsert.empty() && (toInsert.back() & 0xC0) == 0xC0) {
                 toInsert.pop_back(); // Remove the start byte if we cut off its continuation bytes
            }
        }
    }

    // Check Max Lines Limit
    if (m_multiline && m_maxLines > 0) {
        int currentLines = 1;
        for (char c : m_text) {
            if (c == '\n') currentLines++;
        }
        
        int allowedNewlines = m_maxLines - currentLines;
        if (allowedNewlines < 0) allowedNewlines = 0;
        
        int addedNewlines = 0;
        for (int i = 0; i < toInsert.length(); i++) {
            if (toInsert[i] == '\n') {
                if (addedNewlines >= allowedNewlines) {
                    toInsert = toInsert.substr(0, i);
                    break;
                }
                addedNewlines++;
            }
        }
    }

    m_text.insert(m_cursorPos, toInsert);
    m_cursorPos += toInsert.length();
}

int Textbox::GetCursorIndexFromMouse(Vector2 mousePos, Vector2 textBasePos, float scaleY) {
    float scaledFontSize = m_fontSize * scaleY;
    if (m_text.empty()) return 0;

    if (!m_multiline) {
        if (mousePos.x <= textBasePos.x) return 0;
        for (int i = 1; i <= m_text.length(); i++) {
            std::string sub = m_text.substr(0, i);
            float w = MeasureTextEx(m_font, sub.c_str(), scaledFontSize, 1).x;
            if (textBasePos.x + w >= mousePos.x) {
                std::string subPrev = m_text.substr(0, i - 1);
                float wPrev = MeasureTextEx(m_font, subPrev.c_str(), scaledFontSize, 1).x;
                if (mousePos.x - (textBasePos.x + wPrev) < (textBasePos.x + w) - mousePos.x) {
                    return i - 1;
                }
                return i;
            }
        }
        return m_text.length();
    } else {
        // Multi-line logic
        int lineIdx = 0;
        float lineY = textBasePos.y;
        if (mousePos.y < textBasePos.y) mousePos.y = textBasePos.y;
        
        std::vector<std::string> lines;
        std::string currentLine = "";
        for (char c : m_text) {
            if (c == '\n') { lines.push_back(currentLine); currentLine = ""; }
            else { currentLine += c; }
        }
        lines.push_back(currentLine);

        int targetLine = (int)((mousePos.y - textBasePos.y) / (scaledFontSize + 1));
        if (targetLine < 0) targetLine = 0;
        if (targetLine >= lines.size()) targetLine = lines.size() - 1;

        int charsBeforeTargetLine = 0;
        for (int i = 0; i < targetLine; i++) {
            charsBeforeTargetLine += lines[i].length() + 1; // +1 for '\n'
        }

        std::string tLine = lines[targetLine];
        if (mousePos.x <= textBasePos.x) return charsBeforeTargetLine;
        
        for (int i = 1; i <= tLine.length(); i++) {
            std::string sub = tLine.substr(0, i);
            float w = MeasureTextEx(m_font, sub.c_str(), scaledFontSize, 1).x;
            if (textBasePos.x + w >= mousePos.x) {
                std::string subPrev = tLine.substr(0, i - 1);
                float wPrev = MeasureTextEx(m_font, subPrev.c_str(), scaledFontSize, 1).x;
                if (mousePos.x - (textBasePos.x + wPrev) < (textBasePos.x + w) - mousePos.x) {
                    return charsBeforeTargetLine + i - 1;
                }
                return charsBeforeTargetLine + i;
            }
        }
        return charsBeforeTargetLine + tLine.length();
    }
}

void Textbox::HandleInput() {
  bool isShiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  bool isCtrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);

  // Copy/Paste
  if (isCtrlDown) {
      if (IsKeyPressed(KEY_C) && m_selectStart >= 0 && m_selectLength != 0) {
          int start = m_selectLength > 0 ? m_selectStart : m_selectStart + m_selectLength;
          int len = abs(m_selectLength);
          std::string selection = m_text.substr(start, len);
          SetClipboardText(selection.c_str());
      }
      if (IsKeyPressed(KEY_V)) {
          const char* cb = GetClipboardText();
          if (cb) {
              InsertTextAtCursor(std::string(cb));
          }
      }
      // If Ctrl is down, standard typing/navigation might be overridden, but let's let navigation happen (Ctrl+Arrows = jump word is not implemented yet)
  }

  // Arrow Keys Navigation
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
      if (!isShiftDown && m_selectLength != 0) {
          m_cursorPos = m_selectLength > 0 ? m_selectStart : m_selectStart + m_selectLength;
          m_selectStart = -1; m_selectLength = 0;
      } else {
          if (m_selectStart == -1 && isShiftDown) m_selectStart = m_cursorPos;
          
          if (m_cursorPos > 0) {
              m_cursorPos--; // Simple ascii step. Real UTF-8 needs hopping back properly.
              while (m_cursorPos > 0 && (m_text[m_cursorPos] & 0xC0) == 0x80) m_cursorPos--; 
          }
          if (isShiftDown) m_selectLength = m_cursorPos - m_selectStart;
      }
      m_cursorVisible = true;
      m_cursorTimer = 0.0f;
  }
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
      if (!isShiftDown && m_selectLength != 0) {
          m_cursorPos = m_selectLength > 0 ? m_selectStart + m_selectLength : m_selectStart;
          m_selectStart = -1; m_selectLength = 0;
      } else {
          if (m_selectStart == -1 && isShiftDown) m_selectStart = m_cursorPos;
          if (m_cursorPos < m_text.length()) {
              m_cursorPos++;
              while (m_cursorPos < m_text.length() && (m_text[m_cursorPos] & 0xC0) == 0x80) m_cursorPos++;
          }
          if (isShiftDown) m_selectLength = m_cursorPos - m_selectStart;
      }
      m_cursorVisible = true;
      m_cursorTimer = 0.0f;
  }

  // Backspace handler
  if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
    if (m_selectStart >= 0 && m_selectLength != 0) {
        DeleteSelection();
    } else if (m_cursorPos > 0 && !m_text.empty()) {
      int prevPos = m_cursorPos;
      m_cursorPos--;
      while (m_cursorPos > 0 && (m_text[m_cursorPos] & 0xC0) == 0x80) m_cursorPos--;
      int lenToDelete = prevPos - m_cursorPos;
      m_text.erase(m_cursorPos, lenToDelete);
    }
    m_cursorVisible = true; m_cursorTimer = 0.0f;
  }

  // Enter handler
  if (m_multiline && (IsKeyPressed(KEY_ENTER) || IsKeyPressedRepeat(KEY_ENTER))) {
      InsertTextAtCursor("\n");
  }

  // Handle typing characters (ignoring control characters if ctrl is pressed)
  if (!isCtrlDown) {
      int charPressed = GetCharPressed();
      while (charPressed > 0) {
        if ((charPressed >= 32 && charPressed <= 125) || charPressed > 127) {
            int bytes = 0;
            const char *utf8Str = CodepointToUTF8(charPressed, &bytes);
            if (utf8Str) {
              InsertTextAtCursor(std::string(utf8Str, bytes));
            }
        }
        charPressed = GetCharPressed();
      }
  }
}

void Textbox::Draw() {
  Rectangle screenBounds;
  if (m_worldSpace) {
    screenBounds = m_bounds;
  } else {
    screenBounds = Fumbo::Utils::UISpaceToScreen(m_bounds);
  }

  Vector2 scale = {1.0f, 1.0f};
  if (!m_worldSpace) {
      scale = Fumbo::Utils::GetUIScale();
  }

  // 1. Draw Background
  if (m_config.backgroundTexture.id != 0) {
    DrawTexturePro(m_config.backgroundTexture,
                   {0, 0, (float)m_config.backgroundTexture.width,
                    (float)m_config.backgroundTexture.height},
                   screenBounds, {0, 0}, 0.0f, WHITE);
  } else {
    if (m_config.cornerRoundness > 0) {
      DrawRectangleRounded(screenBounds, m_config.cornerRoundness,
                           m_config.cornerSegments, m_config.backgroundColor);
    } else {
      DrawRectangleRec(screenBounds, m_config.backgroundColor);
    }
  }

  // 2. Draw Outline
  Color activeOutlineColor = m_focused ? m_config.focusedOutlineColor : m_config.outlineColor;
  if (m_config.outlineThickness > 0) {
    if (m_config.cornerRoundness > 0) {
      DrawRectangleRoundedLinesEx(screenBounds, m_config.cornerRoundness,
                                m_config.cornerSegments, m_config.outlineThickness * scale.y,
                                activeOutlineColor);
    } else {
      DrawRectangleLinesEx(screenBounds, m_config.outlineThickness * scale.y, activeOutlineColor);
    }
  }

  // 3. Setup text area with padding
  Rectangle textArea = {
      screenBounds.x + (m_config.padding.x * scale.x),
      screenBounds.y + (m_config.padding.y * scale.y),
      screenBounds.width - (m_config.padding.x * scale.x * 2),
      screenBounds.height - (m_config.padding.y * scale.y * 2)
  };

  BeginScissorMode((int)textArea.x, (int)textArea.y, (int)textArea.width, (int)textArea.height);

  Vector2 textPos = {textArea.x, textArea.y};
  float scaledFontSize = m_fontSize * scale.y;
  Vector2 textSize = MeasureTextEx(m_font, m_text.c_str(), scaledFontSize, 1);

  // Single line scrolling & alignment logic
  if (!m_multiline) {
      // Calculate alignment
      if (m_config.textAlignment == TextAlign::CENTER) {
        textPos.x = textArea.x + (textArea.width - textSize.x) / 2.0f;
      } else if (m_config.textAlignment == TextAlign::RIGHT) {
        textPos.x = textArea.x + textArea.width - textSize.x;
      }

      // Scrolling
      if (textSize.x > textArea.width && m_focused) {
        // Keep cursor visible at the end
        m_scrollOffset = textSize.x - textArea.width;
      } else if (!m_focused) {
          m_scrollOffset = 0; // return to beginning when unfocused
      }
      
      textPos.x -= m_scrollOffset;
      textPos.y = textArea.y + (textArea.height - scaledFontSize) / 2.0f;
  }

  // Draw Selection Highlight
  if (m_selectStart >= 0 && m_selectLength != 0) {
      int sStart = m_selectLength > 0 ? m_selectStart : m_selectStart + m_selectLength;
      int sLen = abs(m_selectLength);

      std::string textBefore = m_text.substr(0, sStart);
      std::string textSelected = m_text.substr(sStart, sLen);

      int lineStartIdx = 0;
      int linesBefore = 0;
      for (int i = 0; i < textBefore.length(); i++) {
          if (textBefore[i] == '\n') {
              linesBefore++;
              lineStartIdx = i + 1;
          }
      }

      Vector2 selStartPos = textPos;
      selStartPos.y += linesBefore * (scaledFontSize + 1);
      
      std::string currentLineTextBefore = textBefore.substr(lineStartIdx);
      selStartPos.x += MeasureTextEx(m_font, currentLineTextBefore.c_str(), scaledFontSize, 1).x;

      std::vector<std::string> selLines;
      std::string currentSelLine = "";
      for (char c : textSelected) {
          if (c == '\n') { selLines.push_back(currentSelLine); currentSelLine = ""; }
          else currentSelLine += c;
      }
      selLines.push_back(currentSelLine);

      Vector2 currentDrawPos = selStartPos;
      for (int i = 0; i < selLines.size(); i++) {
          float w = MeasureTextEx(m_font, selLines[i].c_str(), scaledFontSize, 1).x;
          if (selLines[i].empty() && i < selLines.size() - 1) w = 10.0f; // Highlight empty lines
          DrawRectangleRec({currentDrawPos.x, currentDrawPos.y, w, scaledFontSize}, m_config.selectionColor);
          
          currentDrawPos.x = textPos.x;
          currentDrawPos.y += scaledFontSize + 1;
      }
  }

  // Draw actual text
  DrawTextEx(m_font, m_text.c_str(), textPos, scaledFontSize, 1, m_config.textColor);

  // Draw Cursor
  if (m_cursorVisible) {
    Vector2 cursorPos = textPos;
    
    if (m_multiline) {
      // For multiline, we should measure up to cursor, simple approach using substring
      std::string textUpToCursor = m_text.substr(0, m_cursorPos);

      // Find how many newlines
      int lines = 1;
      int lineStartIdx = 0;
      for (int i = 0; i < (int)textUpToCursor.length(); i++) {
        if (textUpToCursor[i] == '\n') {
          lines++;
          lineStartIdx = i + 1;
        }
      }
      
      std::string currentLineText = textUpToCursor.substr(lineStartIdx);
      Vector2 currentLineSize = MeasureTextEx(m_font, currentLineText.c_str(), scaledFontSize, 1);
      
      cursorPos.x += currentLineSize.x;
      cursorPos.y += (lines - 1) * (scaledFontSize + 1); // line spacing offset
      
    } else {
      cursorPos.x += textSize.x;
    }

    DrawLineEx(cursorPos, {cursorPos.x, cursorPos.y + scaledFontSize}, 2.0f * scale.y, m_config.cursorColor);
  }

  EndScissorMode();
}

} // namespace UI
} // namespace Fumbo
