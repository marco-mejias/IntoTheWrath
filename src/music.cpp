#include "music.h"
#include <mmsystem.h>
#include <iostream>
#include <cstdio>
#pragma comment(lib, "winmm.lib")

// ESTRUCTURA OPTIMIZADA: Pre-calculamos los comandos de texto
// para no tener que "sumar textos" mientras juegas.
struct AudioInfo {
    std::wstring alias;
    // Comandos pre-construidos para velocidad máxima
    std::wstring cmdPlay;
    std::wstring cmdPlayRepeat;
    std::wstring cmdSeekStart;
    std::wstring cmdStop;
    bool loaded;
};

std::map<std::wstring, AudioInfo> g_sounds;
int g_idx = 0;

void InitAudio() {
    // Nada crítico
}

void ShutdownAudio() {
    for (auto& s : g_sounds) {
        if (s.second.loaded) {
            // Usamos el comando stop pre-calculado
            mciSendString(L"close all", NULL, 0, NULL);
        }
    }
    g_sounds.clear();
}

bool LoadAudio(const std::wstring& path, const std::wstring& id) {
    if (g_sounds.find(id) != g_sounds.end()) return true;

    // Generamos el alias una sola vez
    std::wstring alias = L"snd_" + std::to_wstring(g_idx++);

    // Abrimos el archivo
    std::wstring openCmd = L"open \"" + path + L"\" type mpegvideo alias " + alias;
    MCIERROR err = mciSendString(openCmd.c_str(), NULL, 0, NULL);

    if (err != 0) {
        // Reintento modo simple
        openCmd = L"open \"" + path + L"\" alias " + alias;
        err = mciSendString(openCmd.c_str(), NULL, 0, NULL);
    }

    if (err == 0) {
        AudioInfo info;
        info.alias = alias;
        info.loaded = true;

        // --- OPTIMIZACIÓN DE RENDIMIENTO ---
        // Pre-cocinamos los comandos para no perder tiempo concatenando strings luego
        info.cmdPlay = L"play " + alias;
        info.cmdPlayRepeat = L"play " + alias + L" repeat";
        info.cmdSeekStart = L"seek " + alias + L" to start";
        info.cmdStop = L"stop " + alias;

        g_sounds[id] = info;

        // Warm-up (Carga en RAM)
        mciSendString(info.cmdSeekStart.c_str(), NULL, 0, NULL);
        mciSendString(info.cmdPlay.c_str(), NULL, 0, NULL);
        mciSendString(info.cmdStop.c_str(), NULL, 0, NULL);

        return true;
    }
    else {
        wchar_t buff[256];
        mciGetErrorString(err, buff, 256);
        fwprintf(stderr, L"[AUDIO LOAD ERROR] %ls -> %ls\n", path.c_str(), buff);
        return false;
    }
}

void SetVolume(const std::wstring& id, int volume) {
    if (g_sounds.find(id) == g_sounds.end()) return;

    // Clamp 0-100
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    // MCI usa rango 0-1000. Multiplicamos por 10.
    int mciVol = volume * 10;

    std::wstring alias = g_sounds[id].alias;

    // Este comando se usa poco, así que podemos construirlo al vuelo
    wchar_t cmd[128];
    swprintf_s(cmd, 128, L"setaudio %s volume to %d", alias.c_str(), mciVol);

    mciSendString(cmd, NULL, 0, NULL);
}

void PlayMusicLoop(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return;
    if (IsPlaying(id)) return;

    // Usamos el comando pre-calculado: MAXIMA VELOCIDAD
    const AudioInfo& info = g_sounds[id];
    mciSendString(info.cmdSeekStart.c_str(), NULL, 0, NULL);
    mciSendString(info.cmdPlayRepeat.c_str(), NULL, 0, NULL);
}

void PlaySoundOnce(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return;

    // Acceso directo a memoria pre-calculada
    const AudioInfo& info = g_sounds[id];

    // Ejecución inmediata sin construir strings
    mciSendString(info.cmdSeekStart.c_str(), NULL, 0, NULL);
    mciSendString(info.cmdPlay.c_str(), NULL, 0, NULL);
}

void StopSound(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return;
    mciSendString(g_sounds[id].cmdStop.c_str(), NULL, 0, NULL);
}

bool IsPlaying(const std::wstring& id) {
    if (g_sounds.find(id) == g_sounds.end()) return false;

    wchar_t status[128];
    // Usamos swprintf para ser rápidos y seguros
    wchar_t cmd[128];
    swprintf_s(cmd, 128, L"status %s mode", g_sounds[id].alias.c_str());

    mciSendString(cmd, status, 128, NULL);
    return (wcscmp(status, L"playing") == 0);
}