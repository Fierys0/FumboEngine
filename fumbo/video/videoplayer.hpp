#pragma once
#include "raylib.h"
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <string>

namespace Fumbo {
namespace Video {

class VideoPlayer {
public:
  bool Init();
  bool Play(const char *path, bool skippable = false);
  void Shutdown();

private:
  mpv_handle *mpv = nullptr;
  mpv_render_context *ctx = nullptr;

  Texture2D texture = {0};
  unsigned char *frameBuffer = nullptr;
  int width = 0;
  int height = 0;
};

} // namespace Video
} // namespace Fumbo
