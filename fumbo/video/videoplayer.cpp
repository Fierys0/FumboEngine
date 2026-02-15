#include "videoplayer.hpp"

#include <cstring>

namespace Fumbo {
namespace Video {

bool VideoPlayer::Init() {
  mpv = mpv_create();
  if (!mpv) {
    TraceLog(LOG_ERROR, "[VideoPlayer] mpv_create failed");
    return false;
  }

  mpv_set_option_string(mpv, "vo", "libmpv");
  mpv_set_option_string(mpv, "hwdec", "no");

  if (mpv_initialize(mpv) < 0) {
    TraceLog(LOG_ERROR, "[VideoPlayer] mpv_initialize failed");
    return false;
  }

  mpv_render_param params[] = {{MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_SW},
                               {MPV_RENDER_PARAM_INVALID, nullptr}};

  int res = mpv_render_context_create(&ctx, mpv, params);
  if (res < 0) {
    TraceLog(LOG_ERROR, "[VideoPlayer] render context create failed");
    return false;
  }

  TraceLog(LOG_INFO, "[VideoPlayer] Initialized (software renderer)");
  return true;
}

bool VideoPlayer::Play(const char* path, bool skippable) {
  // Videos are not encrypted - load directly from file
  if (!FileExists(path)) {
    TraceLog(LOG_WARNING, "[VideoPlayer] Video not found: %s", path);
    return false;
  }
  
  const char* cmd[] = {"loadfile", path, nullptr};
  if (mpv_command(mpv, cmd) < 0) {
    TraceLog(LOG_ERROR, "[VideoPlayer] loadfile failed");
    return false;
  }

  bool playing = true;

  while (playing && !WindowShouldClose()) {
    if (skippable && (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
      const char* stop[] = {"stop", nullptr};
      mpv_command(mpv, stop);
      break;
    }

    mpv_event* ev = mpv_wait_event(mpv, 0);
    if (ev && ev->event_id == MPV_EVENT_END_FILE) break;

    int64_t w = 0, h = 0;
    mpv_get_property(mpv, "width", MPV_FORMAT_INT64, &w);
    mpv_get_property(mpv, "height", MPV_FORMAT_INT64, &h);

    if (w <= 0 || h <= 0) continue;

    if (!frameBuffer) {
      width = (int)w;
      height = (int)h;

      frameBuffer = new unsigned char[width * height * 4];
      texture = LoadTextureFromImage(GenImageColor(width, height, WHITE));
    }

    int size[2] = {width, height};
    int stride = width * 4;

    mpv_render_param render_params[] = {{MPV_RENDER_PARAM_SW_SIZE, size},
                                        {MPV_RENDER_PARAM_SW_FORMAT, (void*)"rgba"},
                                        {MPV_RENDER_PARAM_SW_STRIDE, &stride},
                                        {MPV_RENDER_PARAM_SW_POINTER, frameBuffer},
                                        {MPV_RENDER_PARAM_INVALID, nullptr}};

    mpv_render_context_render(ctx, render_params);

    UpdateTexture(texture, frameBuffer);

    BeginDrawing();
    ClearBackground(BLACK);  // Better for video
    DrawTexturePro(texture, {0, 0, (float)width, (float)height},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}, {0, 0}, 0, WHITE);
    EndDrawing();
  }

  return true;
}

void VideoPlayer::Shutdown() {
  if (ctx) mpv_render_context_free(ctx);

  if (mpv) mpv_terminate_destroy(mpv);

  if (texture.id) UnloadTexture(texture);

  if (frameBuffer) {
    delete[] frameBuffer;
    frameBuffer = nullptr;
  }
}

}  // namespace Video
}  // namespace Fumbo
