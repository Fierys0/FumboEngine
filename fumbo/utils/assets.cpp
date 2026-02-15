#include "../../fumbo.hpp"
#include <memory>
#include <vector>

namespace Fumbo {
  namespace Assets {
    // Global asset packs
    static std::vector<std::unique_ptr<AssetPack>> g_assetPacks;

    void AddAssetPack(const std::string& packPath) {
      auto pack = std::make_unique<AssetPack>();
      if (pack->Load(packPath)) {
        g_assetPacks.push_back(std::move(pack));
      } else {
        TraceLog(LOG_WARNING, "[Assets] Failed to load asset pack: %s", packPath.c_str());
      }
    }

    const std::vector<std::unique_ptr<AssetPack>>& GetAssetPacks() {
      return g_assetPacks;
    }

    Texture2D LoadTexture(const std::string& fileName) {
      // Try loading from packs first
      for (const auto& pack : g_assetPacks) {
        if (pack && pack->IsLoaded() && pack->HasAsset(fileName)) {
          std::vector<uint8_t> data = pack->LoadAsset(fileName);
          if (!data.empty()) {
            const char* ext = GetFileExtension(fileName.c_str());
            Image img = LoadImageFromMemory(ext, data.data(), static_cast<int>(data.size()));
            if (img.data != nullptr) {
              Texture2D tex = LoadTextureFromImage(img);
              UnloadImage(img);
              if (tex.id > 0) {
                TraceLog(LOG_INFO, "[Assets] Loaded texture from pack: %s", fileName.c_str());
                return tex;
              }
            }
          }
        }
      }

      // Fallback to raw file
      if (FileExists(fileName.c_str())) {
        Texture2D tex = ::LoadTexture(fileName.c_str());
        if (tex.id > 0) {
          TraceLog(LOG_INFO, "[Assets] Loaded texture from file: %s", fileName.c_str());
          return tex;
        }
      }

      // Final fallback
      TraceLog(LOG_WARNING, "[Assets] Texture not found: %s", fileName.c_str());
      return CheckedTexture();
    }

    Image LoadImage(const std::string& fileName) {
      // Try loading from packs first
      for (const auto& pack : g_assetPacks) {
        if (pack && pack->IsLoaded() && pack->HasAsset(fileName)) {
          std::vector<uint8_t> data = pack->LoadAsset(fileName);
          if (!data.empty()) {
            const char* ext = GetFileExtension(fileName.c_str());
            Image img = LoadImageFromMemory(ext, data.data(), static_cast<int>(data.size()));
            if (img.data != nullptr) {
              TraceLog(LOG_INFO, "[Assets] Loaded image from pack: %s", fileName.c_str());
              return img;
            }
          }
        }
      }

      // Fallback to raw file
      if (FileExists(fileName.c_str())) {
        Image img = ::LoadImage(fileName.c_str());
        if (img.data != nullptr) {
          TraceLog(LOG_INFO, "[Assets] Loaded image from file: %s", fileName.c_str());
          return img;
        }
      }

      // Final fallback
      TraceLog(LOG_WARNING, "[Assets] Image not found: %s", fileName.c_str());
      return CheckedImage();
    }

    Font LoadFont(const std::string& fileName, int fontSize) {
      // Try loading from packs first
      for (const auto& pack : g_assetPacks) {
        if (pack && pack->IsLoaded() && pack->HasAsset(fileName)) {
          std::vector<uint8_t> data = pack->LoadAsset(fileName);
          if (!data.empty()) {
            const char* ext = GetFileExtension(fileName.c_str());
            Font font = LoadFontFromMemory(ext, data.data(), static_cast<int>(data.size()), fontSize, nullptr, 0);
            if (font.texture.id > 0) {
              TraceLog(LOG_INFO, "[Assets] Loaded font from pack: %s", fileName.c_str());
              return font;
            }
          }
        }
      }

      // Fallback to raw file
      if (FileExists(fileName.c_str())) {
        Font font = LoadFontEx(fileName.c_str(), fontSize, nullptr, 0);
        if (font.texture.id > 0) {
          TraceLog(LOG_INFO, "[Assets] Loaded font from file: %s", fileName.c_str());
          return font;
        }
      }

      // Final fallback - return default font
      TraceLog(LOG_WARNING, "[Assets] Font not found: %s", fileName.c_str());
      return GetFontDefault();
    }

    Sound LoadSound(const std::string& fileName) {
      // Try loading from packs first
      for (const auto& pack : g_assetPacks) {
        if (pack && pack->IsLoaded() && pack->HasAsset(fileName)) {
          std::vector<uint8_t> data = pack->LoadAsset(fileName);
          if (!data.empty()) {
            const char* ext = GetFileExtension(fileName.c_str());
            Wave wave = LoadWaveFromMemory(ext, data.data(), static_cast<int>(data.size()));
            if (wave.data != nullptr) {
              ::Sound sound = LoadSoundFromWave(wave);
              UnloadWave(wave);
              if (sound.stream.buffer != 0) {
                TraceLog(LOG_INFO, "[Assets] Loaded sound from pack: %s", fileName.c_str());
                return sound;
              }
            }
          }
        }
      }

      // Fallback to raw file
      if (FileExists(fileName.c_str())) {
        ::Sound sound = ::LoadSound(fileName.c_str());
        if (sound.stream.buffer != 0) {
          TraceLog(LOG_INFO, "[Assets] Loaded sound from file: %s", fileName.c_str());
          return sound;
        }
      }

      // Final fallback - return empty sound
      TraceLog(LOG_WARNING, "[Assets] Sound not found: %s", fileName.c_str());
      return ::Sound{};
    }

    Music LoadMusic(const std::string& fileName) {
      // Try loading from packs first
      for (const auto& pack : g_assetPacks) {
        if (pack && pack->IsLoaded() && pack->HasAsset(fileName)) {
          std::vector<uint8_t> data = pack->LoadAsset(fileName);
          if (!data.empty()) {
            const char* ext = GetFileExtension(fileName.c_str());
            ::Music music = LoadMusicStreamFromMemory(ext, data.data(), static_cast<int>(data.size()));
            if (music.stream.buffer != 0) {
              TraceLog(LOG_INFO, "[Assets] Loaded music from pack: %s", fileName.c_str());
              return music;
            }
          }
        }
      }

      // Fallback to raw file
      if (FileExists(fileName.c_str())) {
        ::Music music = ::LoadMusicStream(fileName.c_str());
        if (music.stream.buffer != 0) {
          TraceLog(LOG_INFO, "[Assets] Loaded music from file: %s", fileName.c_str());
          return music;
        }
      }

      // Final fallback - return empty music
      TraceLog(LOG_WARNING, "[Assets] Music not found: %s", fileName.c_str());
      return ::Music{};
    }

    Texture2D CheckedTexture() {
      Image checked = GenImageChecked(64, 64, 32, 32, MAGENTA, BLACK);
      Texture2D tex = LoadTextureFromImage(checked);
      UnloadImage(checked);
      return tex;
    }

    Image CheckedImage() {
      return GenImageChecked(64, 64, 32, 32, MAGENTA, BLACK);
    }
  }  // namespace Assets
}  // namespace Fumbo
