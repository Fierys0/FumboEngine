#pragma once
#include "raylib.h"
#include <algorithm>
#include <string>
#include <vector>

class FadeEffect {
public:
  bool active = false;
  int group = 0;

  Texture2D texture{};
  std::string text;
  Font font{};
  bool isText = false;

  Vector2 pos{};    // base position in UI coordinates
  Vector2 size{};   // base size for texture
  float fontSize{}; // base font size for text

  float duration = 1.0f;
  float timer = 0.0f;
  bool fadeIn = true;

  void Start(Texture2D tex, Vector2 pos, Vector2 size, float duration,
             bool fadeIn = true, int group = 0);
  void Start(const std::string &text, Font font, Vector2 pos, float fontSize,
             float duration, bool fadeIn = true, int group = 0);
  void Draw();
  void Reset() {
    timer = 0.0f;
    active = false;
  }
  void Reverse();
  bool IsActive() const;
};

class FadeManager {
private:
  std::vector<FadeEffect> fades;

public:
  FadeManager(int maxFades = 10) { fades.resize(maxFades); }
  FadeEffect *AddFade(Texture2D tex, Vector2 pos, Vector2 size, float duration,
                      bool fadeIn = true, int group = 0);
  FadeEffect *AddFade(const std::string &text, Font font, Vector2 pos,
                      float fontSize, float duration, bool fadeIn = true,
                      int group = 0);
  void Draw(int group = -1);
  void DrawExcept(int excludedGroup);
  void RemoveGroup(int group);
  void Clear();
  void ResetFade(FadeEffect *f);
  void ResetGroup(int groupID);
  void ResetAll();
  void ReverseFade(FadeEffect *f) {
    if (f)
      f->Reverse();
  }
  void ReverseGroup(int groupID) {
    for (auto &f : fades)
      if (f.group == groupID)
        f.Reverse();
  }
  bool IsGroupActive(int groupID) const;
  bool IsGroupFinished(int groupID) const;
};

// extern FadeManager fader; // Removed to prevent confusion. Use
// ShaderManager/Engine instance.
