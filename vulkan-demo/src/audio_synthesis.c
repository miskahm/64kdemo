#include "audio_synthesis.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define PI 3.14159265359f
#define TWO_PI (2.0f * PI)

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

static void audio_update_sequencer(AudioEngine* engine, float dt);

static void audio_data_callback(void* pDevice, void* pOutput, const void* pInput, uint32_t frameCount) {
    ma_device* device = (ma_device*)pDevice;
    AudioEngine* engine = (AudioEngine*)device->config.pUserData;
    float* pOutputF32 = (float*)pOutput;
    
    (void)pInput;
    
    float dt_per_sample = 1.0f / engine->sequencer.sample_rate;
    float bass_sum = 0.0f;
    float mid_sum = 0.0f;
    float high_sum = 0.0f;
    
    for (ma_uint32 i = 0; i < frameCount; i++) {
        audio_update_sequencer(engine, dt_per_sample);
        
        float sample = audio_generate_sample(engine);
        pOutputF32[i*2 + 0] = sample;
        pOutputF32[i*2 + 1] = sample;
        
        float abs_sample = fabsf(sample);
        if (engine->oscillators[0].amplitude > 0.01f) bass_sum += abs_sample;
        if (engine->oscillators[2].amplitude > 0.01f) mid_sum += abs_sample;
        if (engine->oscillators[3].amplitude > 0.01f) high_sum += abs_sample;
    }
    
    for (int i = 0; i < 4; i++) {
        engine->snapshot.oscillators[i] = engine->oscillators[i];
    }
    engine->snapshot.current_pattern = engine->sequencer.current_pattern;
    engine->snapshot.current_row = engine->sequencer.current_row;
    engine->snapshot.bpm = engine->sequencer.bpm;
    engine->snapshot.bass_energy = bass_sum / (float)frameCount;
    engine->snapshot.mid_energy = mid_sum / (float)frameCount;
    engine->snapshot.high_energy = high_sum / (float)frameCount;
}

void audio_init(AudioEngine* engine, float sample_rate) {
    engine->sequencer.sample_rate = sample_rate;
    engine->sequencer.time = 0.0f;
    engine->sequencer.bpm = 140.0f;
    engine->sequencer.playing = true;
    engine->sequencer.current_pattern = 0;
    engine->sequencer.current_row = 0;
    engine->sequencer.pattern_time = 0.0f;
    engine->master_volume = 0.5f;
    engine->filter_cutoff = 2000.0f;
    engine->filter_resonance = 0.5f;
    engine->filter_env = 0.0f;
    engine->device_initialized = false;
    engine->filter_state = 0.0f;
    engine->hihat_accumulator = 0.0f;
    engine->filter_x1 = 0.0f;
    engine->filter_x2 = 0.0f;
    engine->filter_y1 = 0.0f;
    engine->filter_y2 = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        engine->oscillators[i].frequency = 440.0f;
        engine->oscillators[i].amplitude = 0.0f;
        engine->oscillators[i].phase = 0.0f;
        engine->oscillators[i].phase_increment = 0.0f;
    }
    
    engine->snapshot.bass_energy = 0.0f;
    engine->snapshot.mid_energy = 0.0f;
    engine->snapshot.high_energy = 0.0f;
}

void audio_update(AudioEngine* engine, float dt) {
    (void)engine;
    (void)dt;
}

void audio_get_snapshot(AudioEngine* engine, AudioSnapshot* snapshot) {
    for (int i = 0; i < 4; i++) {
        snapshot->oscillators[i] = engine->snapshot.oscillators[i];
    }
    snapshot->current_pattern = engine->snapshot.current_pattern;
    snapshot->current_row = engine->snapshot.current_row;
    snapshot->bpm = engine->snapshot.bpm;
    snapshot->bass_energy = engine->snapshot.bass_energy;
    snapshot->mid_energy = engine->snapshot.mid_energy;
    snapshot->high_energy = engine->snapshot.high_energy;
}

static void audio_update_sequencer(AudioEngine* engine, float dt) {
    engine->sequencer.time += dt;
    
    float row_duration = 60.0f / (engine->sequencer.bpm * 4.0f);
    engine->sequencer.pattern_time += dt;
    
    if (engine->sequencer.pattern_time >= row_duration) {
        engine->sequencer.pattern_time = 0.0f;
        engine->sequencer.current_row++;
        
        if (engine->sequencer.current_row >= 64) {
            engine->sequencer.current_row = 0;
            engine->sequencer.current_pattern = (engine->sequencer.current_pattern + 1) % 8;
        }
        
        int scene = (int)(engine->sequencer.time / 12.0f) % 5;
        float scene_time = fmodf(engine->sequencer.time, 12.0f);
        int row = engine->sequencer.current_row;
        
        float a_minor[] = {220.0f, 246.94f, 261.63f, 293.66f, 329.63f, 349.23f, 392.0f};
        float chord_roots[] = {220.0f, 174.61f, 261.63f, 196.0f};
        int chord_idx = (row / 16) % 4;
        float root = chord_roots[chord_idx];
        
        if (row % 4 == 0) {
            float kick_freq = 55.0f;
            if (scene >= 1) {
                audio_note_on(&engine->oscillators[0], kick_freq, 0.8f);
            }
        }
        
        if (row % 8 == 4 && scene >= 1) {
            audio_note_on(&engine->oscillators[1], 200.0f, 0.3f);
        }
        
        if (scene == 0) {
            if (row % 2 == 0) {
                int arp_pattern[] = {0, 3, 7, 12, 15, 12, 7, 3};
                int arp_step = (row / 2) % 8;
                float arp_freq = root * powf(2.0f, (float)arp_pattern[arp_step] / 12.0f);
                float amp = 0.12f + (scene_time / 12.0f) * 0.08f;
                audio_note_on(&engine->oscillators[2], arp_freq, amp);
            }
            
            if (row == 0 || row == 32) {
                audio_note_on(&engine->oscillators[3], root * 0.5f, 0.18f);
            }
            if (row == 16) {
                audio_note_on(&engine->oscillators[3], root * 0.5f * powf(2.0f, 7.0f/12.0f), 0.15f);
            }
            
            engine->filter_cutoff = 400.0f + scene_time * 200.0f;
        }
        else if (scene == 1) {
            if (row % 2 == 0) {
                int bass_pattern[] = {0, 0, 7, 7, 3, 3, 10, 10};
                int bass_note = bass_pattern[(row / 2) % 8];
                float bass_freq = root * powf(2.0f, (float)bass_note / 12.0f) * 0.5f;
                audio_note_on(&engine->oscillators[2], bass_freq, 0.38f);
            }
            
            if (row % 3 == 0) {
                int melody_notes[] = {12, 14, 15, 17, 19, 17, 15, 14, 12, 10, 12, 15};
                int melody_idx = (row / 3) % 12;
                float melody_freq = a_minor[0] * powf(2.0f, (float)melody_notes[melody_idx] / 12.0f);
                audio_note_on(&engine->oscillators[3], melody_freq, 0.22f);
            }
            
            engine->filter_cutoff = 1400.0f + sinf(scene_time * 2.0f) * 300.0f;
        }
        else if (scene == 2) {
            if (row % 3 == 0) {
                int lead_notes[] = {19, 17, 15, 14, 12, 14, 15, 17, 19, 22, 19, 17};
                int lead_idx = (row / 3) % 12;
                float lead_freq = a_minor[0] * powf(2.0f, (float)lead_notes[lead_idx] / 12.0f);
                audio_note_on(&engine->oscillators[2], lead_freq, 0.25f);
            }
            
            if (row == 0 || row == 16 || row == 32 || row == 48) {
                audio_note_on(&engine->oscillators[3], root * 0.5f, 0.22f);
            }
            if (row == 8 || row == 24 || row == 40 || row == 56) {
                audio_note_on(&engine->oscillators[3], root * 0.5f * powf(2.0f, 5.0f/12.0f), 0.18f);
            }
            
            engine->filter_cutoff = 2200.0f;
        }
        else if (scene == 3) {
            if (row % 2 == 0) {
                int bass_pattern[] = {0, 0, 7, 7, 3, 3, 10, 7};
                int bass_note = bass_pattern[(row / 2) % 8];
                float bass_freq = root * powf(2.0f, (float)bass_note / 12.0f) * 0.5f;
                audio_note_on(&engine->oscillators[2], bass_freq, 0.42f);
            }
            
            if (row >= 32) {
                if (row % 1 == 0) {
                    int arp_notes[] = {0, 3, 7, 12};
                    int arp_idx = row % 4;
                    float arp_freq = root * powf(2.0f, (float)arp_notes[arp_idx] / 12.0f) * 2.0f;
                    float amp = 0.15f + ((float)(row - 32) / 32.0f) * 0.15f;
                    audio_note_on(&engine->oscillators[3], arp_freq, amp);
                }
            } else {
                if (row % 8 == 0) {
                    audio_note_on(&engine->oscillators[3], root * 2.0f, 0.18f);
                }
            }
            
            float buildup = (row >= 32) ? ((float)(row - 32) / 32.0f) : 0.0f;
            engine->filter_cutoff = 900.0f + buildup * 1800.0f;
        }
        else if (scene == 4) {
            if (row % 2 == 0) {
                int bass_pattern[] = {0, 0, 7, 7, 3, 3, 10, 10};
                int bass_note = bass_pattern[(row / 2) % 8];
                float bass_freq = root * powf(2.0f, (float)bass_note / 12.0f) * 0.5f;
                audio_note_on(&engine->oscillators[2], bass_freq, 0.48f);
            }
            
            if (row % 1 == 0) {
                int lead_notes[] = {24, 22, 19, 17, 24, 26, 24, 22, 19, 17, 19, 22, 24, 27, 24, 22};
                int lead_idx = row % 16;
                float lead_freq = a_minor[0] * powf(2.0f, (float)lead_notes[lead_idx] / 12.0f);
                float amp = 0.25f + ((float)row / 64.0f) * 0.15f;
                audio_note_on(&engine->oscillators[3], lead_freq, amp);
            }
            
            float intensity = (float)row / 64.0f;
            engine->filter_cutoff = 2000.0f + intensity * 1500.0f;
        }
        
        engine->filter_env = (row % 4 == 0) ? 1.0f : 0.5f;
    }
}

float audio_generate_sample(AudioEngine* engine) {
    float sample = 0.0f;
    float sample_rate = engine->sequencer.sample_rate;
    float dt = 1.0f / sample_rate;
    
    for (int i = 0; i < 4; i++) {
        Oscillator* osc = &engine->oscillators[i];
        
        if (osc->amplitude > 0.0f) {
            float wave = 0.0f;
            
            if (i == 0) {
                float kick_phase = osc->phase * 0.3f;
                wave = audio_sine(kick_phase) * expf(-osc->phase * 3.0f);
            }
            else if (i == 1) {
                wave = audio_noise() * 0.5f + audio_square(osc->phase * 8.0f) * 0.5f;
            }
            else if (i == 2) {
                float detune1 = audio_sawtooth(osc->phase);
                float detune2 = audio_sawtooth(osc->phase + 0.02f);
                float detune3 = audio_sawtooth(osc->phase - 0.02f);
                wave = (detune1 + detune2 + detune3) / 3.0f;
            }
            else if (i == 3) {
                float pw = 0.5f + 0.3f * audio_sine(osc->phase * 0.1f);
                float pulse = (fmodf(osc->phase, TWO_PI) < (TWO_PI * pw)) ? 1.0f : -1.0f;
                wave = pulse * 0.6f + audio_sine(osc->phase * 2.0f) * 0.4f;
            }
            
            sample += wave * osc->amplitude;
            
            float phase_inc = TWO_PI * osc->phase_increment * dt;
            osc->phase += phase_inc;
            if (osc->phase >= TWO_PI) {
                osc->phase -= TWO_PI;
            }
            
            float decay_rate = (i == 0) ? 0.998f : (i == 1) ? 0.992f : 0.9995f;
            osc->amplitude *= decay_rate;
            if (osc->amplitude < 0.001f) {
                osc->amplitude = 0.0f;
            }
        }
    }
    
    float cutoff = engine->filter_cutoff * (1.0f + engine->filter_env * 0.5f);
    sample = audio_filter(engine, sample, cutoff, engine->filter_resonance);
    
    int row = engine->sequencer.current_row;
    int scene = (int)(engine->sequencer.time / 12.0f) % 5;
    if (scene >= 1 && (row % 2 == 1)) {
        sample += audio_noise() * 0.04f * engine->filter_env;
    }
    
    sample *= engine->master_volume * 0.8f;
    sample = tanhf(sample * 1.2f) * 0.7f;
    
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;
    
    return sample;
}

void audio_note_on(Oscillator* osc, float frequency, float amplitude) {
    osc->frequency = frequency;
    osc->amplitude = amplitude;
    osc->phase = 0.0f;
    osc->phase_increment = frequency;
}

void audio_note_off(Oscillator* osc) {
    osc->amplitude = 0.0f;
}

void audio_set_filter(AudioEngine* engine, float cutoff, float resonance) {
    engine->filter_cutoff = cutoff;
    engine->filter_resonance = resonance;
}

float audio_sine(float phase) {
    return sinf(phase);
}

float audio_square(float phase) {
    float wrapped = fmodf(phase, TWO_PI);
    return (wrapped < PI) ? 1.0f : -1.0f;
}

float audio_sawtooth(float phase) {
    float wrapped = fmodf(phase, TWO_PI);
    return (2.0f * wrapped / TWO_PI) - 1.0f;
}

float audio_noise(void) {
    return (float)rand() / RAND_MAX * 2.0f - 1.0f;
}

float audio_filter(AudioEngine* engine, float input, float cutoff, float resonance) {
    float freq = cutoff / engine->sequencer.sample_rate;
    if (freq > 0.49f) freq = 0.49f;
    if (freq < 0.001f) freq = 0.001f;
    
    float q = resonance;
    
    float d = tanf(PI * freq);
    float c = 1.0f / (1.0f + d * q + d * d);
    
    float a0 = d * d * c;
    float a1 = 2.0f * a0;
    float a2 = a0;
    float b1 = 2.0f * (d * d - 1.0f) * c;
    float b2 = (1.0f - d * q + d * d) * c;
    
    float output = a0 * input + a1 * engine->filter_x1 + a2 * engine->filter_x2 - b1 * engine->filter_y1 - b2 * engine->filter_y2;
    
    engine->filter_x2 = engine->filter_x1;
    engine->filter_x1 = input;
    engine->filter_y2 = engine->filter_y1;
    engine->filter_y1 = output;
    
    return output;
}

int audio_device_init(AudioEngine* engine) {
    ma_device_config config;
    
    config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate        = 44100;
    config.dataCallback      = audio_data_callback;
    config.pUserData         = engine;
    
    ma_result result = ma_device_init(NULL, &config, &engine->device);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "ma_device_init failed: %d\n", result);
        return -1;
    }
    
    engine->device_initialized = true;
    return 0;
}

void audio_device_start(AudioEngine* engine) {
    if (engine->device_initialized) {
        ma_result result = ma_device_start(&engine->device);
        if (result != MA_SUCCESS) {
            fprintf(stderr, "WARNING: ma_device_start failed: %d (audio will not play)\n", result);
        } else {
            printf("Audio device started successfully\n");
        }
    }
}

void audio_device_cleanup(AudioEngine* engine) {
    if (engine->device_initialized) {
        ma_device_uninit(&engine->device);
        engine->device_initialized = false;
    }
}