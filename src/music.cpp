// Media Foundation

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mfplay.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")


#ifndef MFP_EVENT_TYPE_MEDIAITEM_EOF
// Official value from the Windows SDK (MFP_EVENT_TYPE enumeration)
#define MFP_EVENT_TYPE_MEDIAITEM_EOF ((MFP_EVENT_TYPE)0x0014)
#endif

// XAudio2

//#include <xaudio2.h>
//#include <iostream>
//#pragma comment(lib, "mfuuid.lib")
//#pragma comment(lib, "xaudio2.lib")

// Music Header 

#include "music.h"
#include <string>
#include <iostream>

// Global Media Player pointer
static IMFPMediaPlayer* g_player = nullptr;

// Custom callback class to handle media events (e.g., end of playback)
class MusicCallback : public IMFPMediaPlayerCallback
{
    LONG m_refCount = 1;

public:
    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&m_refCount); }
    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG ulRefCount = InterlockedDecrement(&m_refCount);
        if (ulRefCount == 0) delete this;
        return ulRefCount;
    }
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override
    {
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IMFPMediaPlayerCallback))
        {
            *ppv = static_cast<IMFPMediaPlayerCallback*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    // IMFPMediaPlayerCallback
    void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER* pEventHeader) override
    {
        if (pEventHeader->eEventType == MFP_EVENT_TYPE_MEDIAITEM_EOF && g_loopEnabled && g_player)
        {
            // Restart playback when reaching the end
            g_player->Play();
        }
    }
};

static MusicCallback* g_callback = nullptr;

void InitMusicSystem()
{
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize Media Foundation.\n";
    }
    g_callback = new MusicCallback();
}

void PlayMusic(const std::wstring& filepath, bool loop)
{
    g_loopEnabled = loop;
    g_isPaused = false;

    // Release previous player if any
    if (g_player)
    {
        g_player->Shutdown();
        g_player->Release();
        g_player = nullptr;
    }

    HRESULT hr = MFPCreateMediaPlayer(
        filepath.c_str(),
        TRUE,                   // start playback immediately
        0,                      // no special options
        g_callback,             // event callback
        NULL,                   // no video window (audio only)
        &g_player
    );

    if (FAILED(hr))
    {
        std::wcerr << L"Failed to play music: " << filepath << std::endl;
        g_player = nullptr;
    }
}

void PauseMusic()
{
    if (g_player && !g_isPaused)
    {
        g_player->Pause();
        g_isPaused = true;
    }
}

void ResumeMusic()
{
    if (g_player && g_isPaused)
    {
        g_player->Play();
        g_isPaused = false;
    }
}

void StopMusic()
{
    if (g_player)
    {
        g_player->Stop();
        g_player->Shutdown();
        g_player->Release();
        g_player = nullptr;
        g_isPaused = false;
    }
}

void ShutdownMusicSystem()
{
    StopMusic();
    MFShutdown();

    if (g_callback)
    {
        g_callback->Release();
        g_callback = nullptr;
    }
}

void SetMusicVolume(int volume)
{
    g_volume = volume;

    if (g_player)
    {
        // Normalizar a rango [0.0, 1.0]
        float normalized = static_cast<float>(volume) / 100.0f;
        g_player->SetVolume(normalized);
    }
}

