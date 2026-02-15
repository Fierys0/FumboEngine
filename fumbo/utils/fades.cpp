#include "../../fumbo.hpp"

// FadeManager fader;

void FadeEffect::Start(Texture2D tex, Vector2 pos, Vector2 size, float duration,
                       bool fadeIn, int group) {
  this->texture = tex;
  this->pos = pos;
  this->size = size;
  this->duration = duration;
  this->fadeIn = fadeIn;
  this->group = group;
  this->active = true;
  this->isText = false;
  this->timer = 0.0f;
}

// Start fade for text
void FadeEffect::Start(const std::string &text, Font font, Vector2 pos,
                       float fontSize, float duration, bool fadeIn, int group) {
  this->text = text;
  this->font = font;
  this->pos = pos;
  this->fontSize = fontSize;
  this->duration = duration;
  this->fadeIn = fadeIn;
  this->group = group;
  this->active = true;
  this->isText = true;
  this->timer = 0.0f;
}

void FadeEffect::Reverse() {
  fadeIn = !fadeIn; // flip direction
  timer = 0.0f;     // restart timer
  active = true;    // make sure it draws
}

void FadeEffect::Draw() {
  if (!active)
    return; // skip if completely inactive

  // Increment timer
  timer += GetFrameTime();

  // Compute alpha
  float alpha = fadeIn ? (timer / duration) : (1.0f - timer / duration);
  alpha = std::clamp(alpha, 0.0f, 1.0f);

  // Deactivate after fade-out finishes
  if (!fadeIn && timer >= duration) {
    active = false;
    alpha = 0.0f; // ensure fully transparent
  }

  Color color = {255, 255, 255, (unsigned char)(alpha * 255)};
  Vector2 scale = Fumbo::Utils::GetUIScale();

  if (isText) {
    Vector2 scaledPos = {pos.x * scale.x, pos.y * scale.y};
    float scaledFont = fontSize * scale.y;
    DrawTextEx(font, text.c_str(), scaledPos, scaledFont, 1.0f, color);
  } else {
    Rectangle source = {0, 0, (float)texture.width, (float)texture.height};
    Rectangle dest = {pos.x * scale.x, pos.y * scale.y, size.x * scale.x,
                      size.y * scale.y};
    DrawTexturePro(texture, source, dest, {0, 0}, 0.0f, color);
  }
}

// Reset a specific fade by pointer
void FadeManager::ResetFade(FadeEffect *f) {
  if (f)
    f->Reset();
}

// Reset all fades in a group
void FadeManager::ResetGroup(int groupID) {
  for (auto &f : fades) {
    if (f.group == groupID)
      f.Reset();
  }
}

// Reset all fades
void FadeManager::ResetAll() {
  for (auto &f : fades)
    f.Reset();
}

// Add fade for a texture
FadeEffect *FadeManager::AddFade(Texture2D tex, Vector2 pos, Vector2 size,
                                 float duration, bool fadeIn, int group) {
  // First, try to find an **inactive fade in the same group**
  for (auto &f : fades) {
    if (f.group == group && !f.active) {
      f.Start(tex, pos, size, duration, fadeIn, group);
      return &f;
    }
  }

  // Otherwise, use any inactive fade
  for (auto &f : fades) {
    if (!f.active) {
      f.Start(tex, pos, size, duration, fadeIn, group);
      return &f;
    }
  }

  // No available fade slot
  return nullptr;
}

// Add fade for text
FadeEffect *FadeManager::AddFade(const std::string &text, Font font,
                                 Vector2 pos, float fontSize, float duration,
                                 bool fadeIn, int group) {
  for (auto &f : fades) {
    if (!f.active) {
      f.Start(text, font, pos, fontSize, duration, fadeIn, group);
      return &f;
    }
  }
  return nullptr;
}

// Draw all fades (optionally filtered by group)
void FadeManager::Draw(int group) {
  for (auto &f : fades) {
    if (f.active && (group < 0 || f.group == group)) {
      f.Draw();
    }
  }
}

void FadeManager::DrawExcept(int excludedGroup) {
  for (auto &f : fades) {
    if (f.active && f.group != excludedGroup) {
      f.Draw();
    }
  }
}

// Remove all fades of a specific group
void FadeManager::RemoveGroup(int group) {
  for (auto &f : fades) {
    if (f.group == group)
      f.active = false;
  }
}

// Remove all fades
void FadeManager::Clear() {
  for (auto &f : fades)
    f.active = false;
}

bool FadeManager::IsGroupFinished(int groupID) const {
  for (const auto &f : fades) {
    if (f.group == groupID && f.active)
      return false;
  }
  return true;
}

bool FadeManager::IsGroupActive(int groupID) const {
  for (const auto &f : fades) {
    if (f.active && f.group == groupID)
      return true;
  }
  return false;
}

bool FadeEffect::IsActive() const { return active; }
