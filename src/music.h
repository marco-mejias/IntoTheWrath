#ifndef MUSIC_H
#define MUSIC_H

#include <string>
#include <map>
#include <windows.h>

// Identificadores
#define ID_MUSIC  L"bg_music"
#define ID_STEPS  L"fx_steps"
#define ID_FLASH_ON L"fx_flash_on"
#define ID_FLASH_OFF L"fx_flash_off"
#define ID_CHEST L"fx_chest"
#define ID_DOOR L"fx_door"
#define ID_QUACK L"fx_quack"
#define ID_STAIRS L"fx_stairs"
#define ID_ITEM L"fx_item"


// Gestión del sistema
void InitAudio();
void ShutdownAudio();

// Carga de audio
bool LoadAudio(const std::wstring& path, const std::wstring& id);

// --- REPRODUCCIÓN ---

// Reproduce música en bucle (Chequeado manualmente)
void PlayMusicLoop(const std::wstring& id);

// Reproduce UNA VEZ (Optimizado para pasos)
void PlaySoundOnce(const std::wstring& id);

// Detener sonido
void StopSound(const std::wstring& id);

// Estado
bool IsPlaying(const std::wstring& id);

// --- NUEVO: VOLUMEN ---
// volume: 0 a 100
void SetVolume(const std::wstring& id, int volume);

#endif
