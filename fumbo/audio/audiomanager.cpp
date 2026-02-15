#include "../../fumbo.hpp"

namespace Fumbo {
  namespace Audio {
    void AudioManager::Cleanup() {
      for (auto& pair : sounds) UnloadSound(pair.second);
      for (auto& pair : musics) UnloadMusicStream(pair.second);
      sounds.clear();
      musics.clear();
    }

    void AudioManager::Update() {
      if (currentMusic.active) {
        if (musics.find(currentMusic.id) != musics.end()) {
          ::Music& m = musics[currentMusic.id];
          UpdateMusicStream(m);

          // Handle Fading
          if (currentMusic.fadingOut) {
            currentMusic.fadeTimer += GetFrameTime();
            float t = currentMusic.fadeTimer / currentMusic.fadeDuration;
            if (t >= 1.0f) {
              StopMusic(currentMusic.id);  // Done
              return;
            } else {
              // Lerp volume down
              float vol = currentMusic.startVol * (1.0f - t);
              ::SetMusicVolume(m, vol * masterVol);
            }
          }

          if (currentMusic.loop && !currentMusic.fadingOut) {
            float len = GetMusicTimeLength(m);
            float pos = GetMusicTimePlayed(m);

            if (pos >= len - 0.05f)
            {
              float seekPos = (currentMusic.loopStart < 0.0f) ? 0.0f : currentMusic.loopStart;
              SeekMusicStream(m, seekPos);
            }
          }
        }
      }
    }

    bool AudioManager::LoadAudio(const std::string& id, const std::string& path, AudioType type) {
      // Get all asset packs
      const auto& packs = Fumbo::Assets::GetAssetPacks();
      
      if (type == AudioType::SOUND) {
        if (sounds.find(id) != sounds.end()) return true;  // Already loaded
        
        // Try loading from packs first
        for (const auto& pack : packs) {
          if (pack && pack->IsLoaded() && pack->HasAsset(path)) {
            std::vector<uint8_t> data = pack->LoadAsset(path);
            if (!data.empty()) {
              const char* ext = GetFileExtension(path.c_str());
              Wave wave = LoadWaveFromMemory(ext, data.data(), static_cast<int>(data.size()));
              if (wave.data != nullptr) {
                ::Sound s = LoadSoundFromWave(wave);
                UnloadWave(wave);
                if (s.stream.buffer != 0) {
                  sounds[id] = s;
                  TraceLog(LOG_INFO, "[AudioManager] Loaded sound from pack: %s", path.c_str());
                  return true;
                }
              }
            }
          }
        }
        
        // Fallback to raw file
        ::Sound s = ::LoadSound(path.c_str());
        if (s.stream.buffer == 0) return false;
        sounds[id] = s;
        TraceLog(LOG_INFO, "[AudioManager] Loaded sound from file: %s", path.c_str());
      } else {
        if (musics.find(id) != musics.end()) return true;
        
        // Try loading from packs first
        for (const auto& pack : packs) {
          if (pack && pack->IsLoaded() && pack->HasAsset(path)) {
            std::vector<uint8_t> data = pack->LoadAsset(path);
            if (!data.empty()) {
              const char* ext = GetFileExtension(path.c_str());
              ::Music m = LoadMusicStreamFromMemory(ext, data.data(), static_cast<int>(data.size()));
              if (m.stream.buffer != 0) {
                m.looping = false;  // We handle looping manually for custom points
                musics[id] = m;
                TraceLog(LOG_INFO, "[AudioManager] Loaded music from pack: %s", path.c_str());
                return true;
              }
            }
          }
        }
        
        // Fallback to raw file
        ::Music m = ::LoadMusicStream(path.c_str());
        if (m.stream.buffer == 0) return false;
        m.looping = false;  // We handle looping manually for custom points
        musics[id] = m;
        TraceLog(LOG_INFO, "[AudioManager] Loaded music from file: %s", path.c_str());
      }
      return true;
    }

    void AudioManager::UnloadAudio(const std::string& id) {
      if (sounds.find(id) != sounds.end()) {
        UnloadSound(sounds[id]);
        sounds.erase(id);
      }
      if (musics.find(id) != musics.end()) {
        UnloadMusicStream(musics[id]);
        musics.erase(id);
      }
    }

    void AudioManager::PlaySound(const std::string& id) {
      if (sounds.find(id) != sounds.end()) {
        ::Sound& s = sounds[id];
        ::SetSoundVolume(s, soundVol * masterVol);
        ::PlaySound(s);
      } else {
        if (LoadAudio(id, id, AudioType::SOUND)) {
          PlaySound(id);
        }
      }
    }

    void AudioManager::StopSound(const std::string& id) {
      if (sounds.find(id) != sounds.end()) {
        ::StopSound(sounds[id]);
      }
    }

    void AudioManager::PlayMusic(const std::string& id, bool loop, float loopStart) {
      // Stop current if any (Basic single channel music)
      if (currentMusic.active && currentMusic.id != id) {
        StopMusic(currentMusic.id);
      }

      if (musics.find(id) == musics.end()) {
        if (!LoadAudio(id, id, AudioType::MUSIC)) return;
      }

      ::Music& m = musics[id];
      ::SetMusicVolume(m, musicVol * masterVol);
      ::PlayMusicStream(m);

      currentMusic.id = id;
      currentMusic.loop = loop;
      currentMusic.loopStart = loopStart;
      currentMusic.active = true;
      currentMusic.fadingOut = false;
      currentMusic.fadeTimer = 0.0f;
      currentMusic.startVol = musicVol;  // Current settings music vol
    }

    void AudioManager::StopMusic(const std::string& id) {
      if (musics.find(id) != musics.end()) {
        ::StopMusicStream(musics[id]);
      }
      if (currentMusic.id == id) {
        currentMusic.active = false;
        currentMusic.fadingOut = false;
      }
    }

    void AudioManager::StopMusicFade(float duration) {
      if (currentMusic.active && !currentMusic.fadingOut) {
        currentMusic.fadingOut = true;
        currentMusic.fadeDuration = duration;
        currentMusic.fadeTimer = 0.0f;
        currentMusic.startVol = musicVol;  // Store current music volume
      }
    }

    void AudioManager::StopAllMusic() {
      if (currentMusic.active) StopMusic(currentMusic.id);
    }

    void AudioManager::SetMasterVolume(float vol) { masterVol = vol; }
    void AudioManager::SetMusicVolume(float vol) {
      musicVol = vol;
      if (currentMusic.active) ::SetMusicVolume(musics[currentMusic.id], musicVol * masterVol);
    }
    void AudioManager::SetSoundVolume(float vol) { soundVol = vol; }
  }  // namespace Audio
}  // namespace Fumbo
