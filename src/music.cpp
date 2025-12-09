#include "music.h"
#include <mmsystem.h>
#include <iostream>
#include <cstdio>
#include <cmath> 
#include <cwchar> // Necesario para wcscmp y funciones wide
#pragma comment(lib, "winmm.lib")

// Definición de variables globales compartidas (Implementadas en music.cpp)
int g_musicVolume = 100;
int g_sfxVolume = 100;

const std::wstring SFX_IDS[] = {
    ID_STEPS, ID_ITEM, ID_STAIRS, ID_CHEST, ID_QUACK,
    ID_DOOR, ID_FLASH_ON, ID_FLASH_OFF, ID_GUN, ID_OPEN_BOOK, ID_CLOSE_BOOK, ID_MENU_HOVER, ID_MENU_SELECT
};
const size_t SFX_COUNT = sizeof(SFX_IDS) / sizeof(SFX_IDS[0]);


// Estructura que guarda la información del audio
struct AudioInfo {
    std::wstring alias;
    int baseVolume; // NUEVO: Volumen base (0-100)
    bool loaded;

    // --- CAMPOS RESTAURADOS PARA OPTIMIZACIÓN DE RENDIMIENTO ---
    std::wstring cmdPlay;
    std::wstring cmdPlayRepeat;
    std::wstring cmdSeekStart;
    std::wstring cmdStop;
};

std::map<std::wstring, AudioInfo> g_sounds;
int g_idx = 0;

void InitAudio() {
    // Nada crítico
}

void ShutdownAudio() {
    for (auto& s : g_sounds) {
        if (s.second.loaded) {
            std::wstring cmd = L"close " + s.second.alias;
            mciSendString(cmd.c_str(), NULL, 0, NULL);
        }
    }
    g_sounds.clear();
}

// MODIFICADA: Ahora acepta el volumen base
bool LoadAudio(const std::wstring& path, const std::wstring& id, int defaultVolume) {

    if (g_sounds.find(id) != g_sounds.end()) return true;

    std::wstring alias = L"snd_" + std::to_wstring(g_idx++);

    // Comando OPEN
    std::wstring openCmd = L"open \"" + path + L"\" type mpegvideo alias " + alias;
    MCIERROR err = mciSendString(openCmd.c_str(), NULL, 0, NULL);

    if (err != 0) {
        openCmd = L"open \"" + path + L"\" alias " + alias;
        err = mciSendString(openCmd.c_str(), NULL, 0, NULL);
    }

    if (err == 0) {
        AudioInfo info;
        info.alias = alias;
        info.loaded = true;
        info.baseVolume = defaultVolume; // GUARDAMOS EL VOLUMEN BASE

        // --- OPTIMIZACIÓN DE RENDIMIENTO: Pre-calculamos los comandos ---
        info.cmdPlay = L"play " + alias;
        info.cmdPlayRepeat = L"play " + alias + L" repeat";
        info.cmdSeekStart = L"seek " + alias + L" to start";
        info.cmdStop = L"stop " + alias;

        g_sounds[id] = info;

        // Warm-up: Aseguramos que el audio se carga en RAM
        mciSendString(info.cmdSeekStart.c_str(), NULL, 0, NULL);

        // Aplicar el volumen inicial (Base * Master)
        SetVolume(id, (id == ID_MUSIC) ? g_musicVolume : g_sfxVolume);

        return true;
    }
    else {
        wchar_t buff[256];
        mciGetErrorString(err, buff, 256);
        fwprintf(stderr, L"[AUDIO LOAD ERROR] %ls -> %ls\n", path.c_str(), buff);
        return false;
    }
}

// MODIFICADA: Aplica (Volumen Base * Volumen Máster) / 100
void SetVolume(const std::wstring& id, int masterVolume) {
    if (g_sounds.find(id) == g_sounds.end()) return;

    AudioInfo& info = g_sounds[id];

    // Fórmula clave: (Base * Máster) / 100
    int finalVolume = (info.baseVolume * masterVolume) / 100;

    if (finalVolume < 0) finalVolume = 0;
    if (finalVolume > 100) finalVolume = 100;

    // MCI usa rango 0-1000
    int mciVol = finalVolume * 10;

    wchar_t cmd[128];
    swprintf_s(cmd, 128, L"setaudio %s volume to %d", info.alias.c_str(), mciVol);

    mciSendString(cmd, NULL, 0, NULL);
}

// NUEVA: Reproduce música en bucle
void PlayMusicLoop(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return;
    // Solo reproducimos si no está sonando ya
    if (IsPlaying(id)) return;

    const AudioInfo& info = g_sounds[id];

    // Usamos comandos pre-calculados (cmdSeekStart, cmdPlayRepeat)
    mciSendString(info.cmdSeekStart.c_str(), NULL, 0, NULL);
    mciSendString(info.cmdPlayRepeat.c_str(), NULL, 0, NULL);
}

// NUEVA: Reproduce UNA VEZ (Para pasos y efectos)
void PlaySoundOnce(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return;

    const AudioInfo& info = g_sounds[id];

    // Usamos comandos pre-calculados (cmdSeekStart, cmdPlay)
    // Esto asegura que el sonido se dispare instantáneamente desde el inicio.
    mciSendString(info.cmdSeekStart.c_str(), NULL, 0, NULL);
    mciSendString(info.cmdPlay.c_str(), NULL, 0, NULL);
}

// NUEVA: Detiene el sonido inmediatamente
void StopSound(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return;
    const AudioInfo& info = g_sounds[id];
    mciSendString(info.cmdStop.c_str(), NULL, 0, NULL);
}

// NUEVA: Consulta si está sonando
bool IsPlaying(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return false;

    const AudioInfo& info = g_sounds[id];

    wchar_t status[128];
    wchar_t cmd[128];
    swprintf_s(cmd, 128, L"status %s mode", info.alias.c_str());

    if (mciSendString(cmd, status, 128, NULL) == 0) {
        // La respuesta estándar de MCI para saber si está sonando
        return (wcscmp(status, L"playing") == 0);
    }
    return false;
}