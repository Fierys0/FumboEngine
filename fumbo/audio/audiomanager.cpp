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

      // Normalize path separators (Windows backslash -> forward slash)
      // miniaudio (used by Raylib) can fail silently on backslash paths
      std::string normPath = path;
      for (char& c : normPath)
        if (c == '\\')
          c = '/';

      Fumbo::Log::Infof("[Audio] LoadAudio id='%s' path='%s'", id.c_str(), normPath.c_str());

      if (type == AudioType::SOUND) {
        if (sounds.find(id) != sounds.end()) {
          Fumbo::Log::Debugf("[Audio] Sound '%s' already loaded, skipping", id.c_str());
          return true;
        }

        // Try loading from packs first
        for (const auto& pack : packs) {
          if (pack && pack->IsLoaded() && pack->HasAsset(normPath)) {
            std::vector<uint8_t> data = pack->LoadAsset(normPath);
            if (!data.empty()) {
              const char* ext = GetFileExtension(normPath.c_str());
              Wave wave = LoadWaveFromMemory(ext, data.data(), static_cast<int>(data.size()));
              if (wave.data != nullptr) {
                ::Sound s = LoadSoundFromWave(wave);
                UnloadWave(wave);
                if (s.stream.buffer != 0) {
                  sounds[id] = s;
                  Fumbo::Log::Infof("[Audio] Sound '%s' loaded from pack", id.c_str());
                  return true;
                }
              }
            }
          }
        }

        // Fallback to raw file
        ::Sound s = ::LoadSound(normPath.c_str());
        if (s.stream.buffer == 0) {
          Fumbo::Log::Errorf("[Audio] Failed to load sound from file: '%s'", normPath.c_str());
          return false;
        }
        sounds[id] = s;
        Fumbo::Log::Infof("[Audio] Sound '%s' loaded from file", id.c_str());
      } else {
        if (musics.find(id) != musics.end()) {
          Fumbo::Log::Debugf("[Audio] Music '%s' already loaded, skipping", id.c_str());
          return true;
        }

        // Try loading from packs first
        for (const auto& pack : packs) {
          if (pack && pack->IsLoaded() && pack->HasAsset(normPath)) {
            std::vector<uint8_t> data = pack->LoadAsset(normPath);
            if (!data.empty()) {
              const char* ext = GetFileExtension(normPath.c_str());
              ::Music m = LoadMusicStreamFromMemory(ext, data.data(), static_cast<int>(data.size()));
              if (m.stream.buffer != 0) {
                m.looping = false;
                musics[id] = m;
                Fumbo::Log::Infof("[Audio] Music '%s' loaded from pack", id.c_str());
                return true;
              }
            }
          }
        }

        // Fallback to raw file
        ::Music m = ::LoadMusicStream(normPath.c_str());
        if (m.stream.buffer == 0) {
          Fumbo::Log::Errorf("[Audio] Failed to load music from file: '%s'", normPath.c_str());
          return false;
        }
        m.looping = false;
        musics[id] = m;
        Fumbo::Log::Infof("[Audio] Music '%s' loaded from file", id.c_str());
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
        MusicState& existing = activeMusics[channel];
        // If same track already active on this channel, just seek to start
        if (existing.id == id && musics.find(id) != musics.end()) {
          Fumbo::Log::Debugf("[Audio] Re-playing same track '%s' on channel %d — seeking to 0", id.c_str(), channel);
          ::Music& m = musics[id];
          ::StopMusicStream(m);
          ::SeekMusicStream(m, 0.0f);  // reset decoder cursor
          float cVol = GetChannelVolume(channel);
          ::SetMusicVolume(m, cVol * masterVol);
          ::PlayMusicStream(m);
          existing.loop      = loop;
          existing.loopStart = loopStart;
          existing.fadingOut = false;
          existing.fadeTimer = 0.0f;
          existing.startVol  = cVol;
          existing.active    = true;
          Fumbo::Log::Infof("[Audio] PlayMusic (seek-restart) '%s' on channel %d", id.c_str(), channel);
          return;
        }
        // Different track — stop old one cleanly
        Fumbo::Log::Debugf("[Audio] Stopping existing music on channel %d before playing '%s'", channel, id.c_str());
        StopMusic(channel);
      }

      if (musics.find(id) == musics.end()) {
        Fumbo::Log::Warnf("[Audio] Music '%s' not in map, attempting auto-load", id.c_str());
        if (!LoadAudio(id, id, AudioType::MUSIC)) {
          Fumbo::Log::Errorf("[Audio] PlayMusic: auto-load failed for '%s', aborting", id.c_str());
          return;
        }
      }

      ::Music& m = musics[id];

      // Always seek to 0 before playing to reset the miniaudio decoder cursor.
      // Without this, PlayMusicStream on a previously-stopped stream produces
      // no audio on some backends (Windows/Android miniaudio).
      ::SeekMusicStream(m, 0.0f);

      float cVol = GetChannelVolume(channel);
      ::SetMusicVolume(m, cVol * masterVol);
      ::PlayMusicStream(m);

      MusicState state;
      state.id        = id;
      state.loop      = loop;
      state.loopStart = loopStart;
      state.active    = true;
      state.fadingOut = false;
      state.fadeTimer = 0.0f;
      state.startVol  = cVol;

      activeMusics[channel] = state;
      Fumbo::Log::Infof("[Audio] PlayMusic '%s' on channel %d  loop=%s  vol=%.2f",
                        id.c_str(), channel, loop ? "true" : "false", cVol * masterVol);
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

    void AudioManager::PauseMusic(int channel) {
      auto it = activeMusics.find(channel);
      if (it != activeMusics.end() && it->second.active) {
        auto mIt = musics.find(it->second.id);
        if (mIt != musics.end()) {
          ::PauseMusicStream(mIt->second);
        }
      }
    }

    void AudioManager::ResumeMusic(int channel) {
      auto it = activeMusics.find(channel);
      if (it != activeMusics.end() && it->second.active) {
        auto mIt = musics.find(it->second.id);
        if (mIt != musics.end()) {
          ::ResumeMusicStream(mIt->second);
        }
      }
    }

    float AudioManager::GetMusicLength(int channel) {
      auto it = activeMusics.find(channel);
      if (it != activeMusics.end() && it->second.active) {
        auto mIt = musics.find(it->second.id);
        if (mIt != musics.end()) {
          return ::GetMusicTimeLength(mIt->second);
        }
      }
      return 0.0f;
    }

    float AudioManager::GetMusicPlayed(int channel) {
      auto it = activeMusics.find(channel);
      if (it != activeMusics.end() && it->second.active) {
        auto mIt = musics.find(it->second.id);
        if (mIt != musics.end()) {
          return ::GetMusicTimePlayed(mIt->second);
        }
      }
      return 0.0f;
    }

    void AudioManager::SeekMusic(float position, int channel) {
      auto it = activeMusics.find(channel);
      if (it != activeMusics.end() && it->second.active) {
        auto mIt = musics.find(it->second.id);
        if (mIt != musics.end()) {
          ::SeekMusicStream(mIt->second, position);
        }
      }
    }

    bool AudioManager::IsMusicPlaying(int channel) {
      auto it = activeMusics.find(channel);
      if (it != activeMusics.end() && it->second.active) {
        auto mIt = musics.find(it->second.id);
        if (mIt != musics.end()) {
          return ::IsMusicStreamPlaying(mIt->second);
        }
      }
      return false;
    }
  }  // namespace Audio
}  // namespace Fumbo
