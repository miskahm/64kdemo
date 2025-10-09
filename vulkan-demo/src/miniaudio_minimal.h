#ifndef MINIAUDIO_MINIMAL_H
#define MINIAUDIO_MINIMAL_H

#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif

typedef uint32_t ma_uint32;

#define MA_SUCCESS 0
#define MA_ERROR -1

typedef int ma_result;
typedef void (*ma_device_callback_proc)(void* pDevice, void* pOutput, const void* pInput, uint32_t frameCount);

typedef enum {
    ma_device_type_playback
} ma_device_type;

typedef enum {
    ma_format_f32
} ma_format;

typedef struct {
    ma_device_type deviceType;
    struct {
        ma_format format;
        uint32_t channels;
    } playback;
    uint32_t sampleRate;
    ma_device_callback_proc dataCallback;
    void* pUserData;
} ma_device_config;

typedef struct {
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeaders[2];
    uint8_t* pBuffers[2];
    uint32_t bufferSize;
    uint32_t currentBuffer;
    ma_device_config config;
    volatile int running;
    HANDLE hThread;
    void* pUserData;
} ma_device;

static ma_device_config ma_device_config_init(ma_device_type deviceType) {
    ma_device_config config;
    config.deviceType = deviceType;
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = 44100;
    config.dataCallback = NULL;
    config.pUserData = NULL;
    return config;
}

static DWORD WINAPI audio_thread(LPVOID lpParam) {
    ma_device* pDevice = (ma_device*)lpParam;
    
    while (pDevice->running) {
        WAVEHDR* pHeader = &pDevice->waveHeaders[pDevice->currentBuffer];
        
        if (pHeader->dwFlags & WHDR_DONE) {
            waveOutUnprepareHeader(pDevice->hWaveOut, pHeader, sizeof(WAVEHDR));
            
            if (pDevice->config.dataCallback) {
                pDevice->config.dataCallback(pDevice, pHeader->lpData, NULL, 
                    pDevice->bufferSize / (sizeof(float) * pDevice->config.playback.channels));
            }
            
            waveOutPrepareHeader(pDevice->hWaveOut, pHeader, sizeof(WAVEHDR));
            waveOutWrite(pDevice->hWaveOut, pHeader, sizeof(WAVEHDR));
            
            pDevice->currentBuffer = (pDevice->currentBuffer + 1) % 2;
        }
        
        Sleep(10);
    }
    
    return 0;
}

static ma_result ma_device_init(void* pContext, const ma_device_config* pConfig, ma_device* pDevice) {
    (void)pContext;
    
    pDevice->config = *pConfig;
    pDevice->bufferSize = 2048 * sizeof(float) * pConfig->playback.channels;
    pDevice->currentBuffer = 0;
    pDevice->running = 0;
    
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wfx.nChannels = pConfig->playback.channels;
    wfx.nSamplesPerSec = pConfig->sampleRate;
    wfx.wBitsPerSample = 32;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;
    
    if (waveOutOpen(&pDevice->hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        return MA_ERROR;
    }
    
    for (int i = 0; i < 2; i++) {
        pDevice->pBuffers[i] = (uint8_t*)malloc(pDevice->bufferSize);
        if (!pDevice->pBuffers[i]) {
            waveOutClose(pDevice->hWaveOut);
            return MA_ERROR;
        }
        
        memset(pDevice->pBuffers[i], 0, pDevice->bufferSize);
        
        pDevice->waveHeaders[i].lpData = (LPSTR)pDevice->pBuffers[i];
        pDevice->waveHeaders[i].dwBufferLength = pDevice->bufferSize;
        pDevice->waveHeaders[i].dwFlags = WHDR_DONE;
        pDevice->waveHeaders[i].dwLoops = 0;
    }
    
    return MA_SUCCESS;
}

static ma_result ma_device_start(ma_device* pDevice) {
    pDevice->running = 1;
    
    for (int i = 0; i < 2; i++) {
        waveOutPrepareHeader(pDevice->hWaveOut, &pDevice->waveHeaders[i], sizeof(WAVEHDR));
        waveOutWrite(pDevice->hWaveOut, &pDevice->waveHeaders[i], sizeof(WAVEHDR));
    }
    
    pDevice->hThread = CreateThread(NULL, 0, audio_thread, pDevice, 0, NULL);
    if (!pDevice->hThread) {
        pDevice->running = 0;
        return MA_ERROR;
    }
    
    return MA_SUCCESS;
}

static void ma_device_uninit(ma_device* pDevice) {
    if (pDevice->running) {
        pDevice->running = 0;
        WaitForSingleObject(pDevice->hThread, INFINITE);
        CloseHandle(pDevice->hThread);
    }
    
    waveOutReset(pDevice->hWaveOut);
    
    for (int i = 0; i < 2; i++) {
        if (pDevice->waveHeaders[i].dwFlags & WHDR_PREPARED) {
            waveOutUnprepareHeader(pDevice->hWaveOut, &pDevice->waveHeaders[i], sizeof(WAVEHDR));
        }
        free(pDevice->pBuffers[i]);
    }
    
    waveOutClose(pDevice->hWaveOut);
}

#endif
