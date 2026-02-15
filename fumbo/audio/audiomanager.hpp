#pragma once
#include <map>
#include <string>

#include "raylib.h"

namespace Fumbo {
  namespace Audio {
    enum class AudioType { SOUND, MUSIC };

    class AudioManager {
    public:
      static AudioManager& Instance() {
        static AudioManager instance;
        return instance;
      }

      void Cleanup();
      void Update();  // Required for looping music manually

      // Sound Effects
      void PlaySound(const std::string& id);
      void StopSound(const std::string& id);

      // Music
      // loopStart: offset in seconds to loop back to when music ends.
      // If loopStart < 0, it loops from beginning.
      // If looping is false, it plays once.
      void PlayMusic(const std::string& id, bool loop = true, float loopStart = 0.0f);
      void StopMusic(const std::string& id);
      void StopMusicFade(float duration = 1.0f);  // Fade out current music
      void StopAllMusic();

      // Resource Loading (Manual loading if desired, or auto-load via Play)
      bool LoadAudio(const std::string& id, const std::string& path, AudioType type);
      void UnloadAudio(const std::string& id);

      // Volume Control (0.0f to 1.0f)
      void SetMasterVolume(float vol);
      void SetMusicVolume(float vol);
      void SetSoundVolume(float vol);

      float GetMasterVolume() const { return masterVol; }
      float GetMusicVolume() const { return musicVol; }
      float GetSoundVolume() const { return soundVol; }

    private:
      AudioManager() = default;
      ~AudioManager() = default;

      std::map<std::string, ::Sound> sounds;
      std::map<std::string, ::Music> musics;

      // Track active music state for custom looping
      struct MusicState {
        std::string id;
        bool loop;
        float loopStart;
        bool active;

        // Fading
        bool fadingOut;
        float fadeDuration;
        float fadeTimer;
        float startVol;
      };

      MusicState currentMusic = {"", false, 0.0f, false, false, 0.0f, 0.0f, 1.0f};

      float masterVol = 1.0f;
      float musicVol = 1.0f;
      float soundVol = 1.0f;
    };
  }  // namespace Audio
}  // namespace Fumbo
