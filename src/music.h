#ifndef MUSIC_H
#define MUSIC_H

#include <string>

static int g_volume = 100; // Volumen del juego
static bool g_loopEnabled = false;
static bool g_isPaused = false;

// Initializes Media Foundation (must be called once before using any music functions)
void InitMusicSystem();

// Plays a music track (e.g. MP3 file). If already playing, restarts it.
void PlayMusic(const std::wstring& filepath, bool loop = true);

// Stops and releases the music player
void StopMusic();

// Shuts down Media Foundation (call when closing your program)
void ShutdownMusicSystem();

// Pauses playback (keeps current position)
void PauseMusic();

// Resumes playback from current position
void ResumeMusic();    

void SetMusicVolume(int volume);
#endif
