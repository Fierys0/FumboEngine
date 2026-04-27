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
      for (auto it = activeMusics.begin(); it != activeMusics.end(); ) {
        MusicState& state = it->second;
        if (state.active) {
          if (musics.find(state.id) != musics.end()) {
            ::Music& m = musics[state.id];
            UpdateMusicStream(m);

            // Handle Fading
            if (state.fadingOut) {
              state.fadeTimer += GetFrameTime();
              float t = state.fadeTimer / state.fadeDuration;
              if (t >= 1.0f) {
                ::StopMusicStream(m);
                it = activeMusics.erase(it);
                continue;
              } else {
                // Lerp volume down
                float vol = state.startVol * (1.0f - t);
                ::SetMusicVolume(m, vol * masterVol);
              }
            } else {
              if (state.loop) {
                float len = GetMusicTimeLength(m);
                float pos = GetMusicTimePlayed(m);

                if (pos >= len - 0.05f)
                {
                  float seekPos = (state.loopStart < 0.0f) ? 0.0f : state.loopStart;
                  SeekMusicStream(m, seekPos);
                }
              }
            }
          } else {
            // Music resource no longer exists, but state is active?
            it = activeMusics.erase(it);
            continue;
          }
        } else {
           it = activeMusics.erase(it);
           continue;
        }
        ++it;
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

    void AudioManager::PlaySound(const std::string& id, int channel) {
      if (sounds.find(id) != sounds.end()) {
        ::Sound& s = sounds[id];
        float cVol = GetChannelVolume(channel);
        ::SetSoundVolume(s, cVol * masterVol);
        ::PlaySound(s);
      } else {
        if (LoadAudio(id, id, AudioType::SOUND)) {
          PlaySound(id, channel);
        }
      }
    }

    void AudioManager::StopSound(const std::string& id) {
      if (sounds.find(id) != sounds.end()) {
        ::StopSound(sounds[id]);
      }
    }

    void AudioManager::PlayMusic(const std::string& id, int channel, bool loop, float loopStart) {
      // Stop current music on this channel if any
      if (activeMusics.find(channel) != activeMusics.end()) {
        StopMusic(channel);
      }

      if (musics.find(id) == musics.end()) {
        if (!LoadAudio(id, id, AudioType::MUSIC)) return;
      }

      ::Music& m = musics[id];
      float cVol = GetChannelVolume(channel);
      ::SetMusicVolume(m, cVol * masterVol);
      ::PlayMusicStream(m);

      MusicState state;
      state.id = id;
      state.loop = loop;
      state.loopStart = loopStart;
      state.active = true;
      state.fadingOut = false;
      state.fadeTimer = 0.0f;
      state.startVol = cVol;

      activeMusics[channel] = state;
    }

    void AudioManager::StopMusic(int channel) {
      if (activeMusics.find(channel) != activeMusics.end()) {
        std::string id = activeMusics[channel].id;
        if (musics.find(id) != musics.end()) {
          ::StopMusicStream(musics[id]);
        }
        activeMusics.erase(channel);
      }
    }

    void AudioManager::StopMusicFade(int channel, float duration) {
      if (activeMusics.find(channel) != activeMusics.end()) {
        MusicState& state = activeMusics[channel];
        if (!state.fadingOut) {
          state.fadingOut = true;
          state.fadeDuration = duration;
          state.fadeTimer = 0.0f;
          state.startVol = GetChannelVolume(channel);
        }
      }
    }

    void AudioManager::ClearChannel(int channel) {
      StopMusic(channel);
    }

    void AudioManager::StopAllMusic() {
      for (auto& pair : activeMusics) {
        if (musics.find(pair.second.id) != musics.end()) {
          ::StopMusicStream(musics[pair.second.id]);
        }
      }
      activeMusics.clear();
    }

    void AudioManager::SetMasterVolume(float vol) { masterVol = vol; }

    void AudioManager::SetChannelVolume(int channel, float vol) {
      channelVolumes[channel] = vol;
      // Update volume immediately if music is playing on this channel and not fading out
      if (activeMusics.find(channel) != activeMusics.end()) {
        MusicState& state = activeMusics[channel];
        if (!state.fadingOut && musics.find(state.id) != musics.end()) {
           ::SetMusicVolume(musics[state.id], vol * masterVol);
           state.startVol = vol;
        }
      }
    }

    float AudioManager::GetChannelVolume(int channel) const {
      auto it = channelVolumes.find(channel);
      return (it != channelVolumes.end()) ? it->second : 1.0f;
    }
  }  // namespace Audio
}  // namespace Fumbo
