#ifndef AUDIO_SYNTHESIS_H
#define AUDIO_SYNTHESIS_H

#include <stdint.h>
#include <stdbool.h>

#include "miniaudio_minimal.h"

typedef struct {
    float frequency;
    float amplitude;
    float phase;
    float phase_increment;
} Oscillator;

typedef struct {
    float time;
    float sample_rate;
    float bpm;
    bool playing;
    int current_pattern;
    int current_row;
    float pattern_time;
} Sequencer;

typedef struct {
    Oscillator oscillators[4];
    int current_pattern;
    int current_row;
    float bpm;
    float bass_energy;
    float mid_energy;
    float high_energy;
} AudioSnapshot;

typedef struct {
    Oscillator oscillators[4];
    Sequencer sequencer;
    float master_volume;
    float filter_cutoff;
    float filter_resonance;
    float filter_env;
    ma_device device;
    bool device_initialized;
    float filter_state;
    float hihat_accumulator;
    float filter_x1;
    float filter_x2;
    float filter_y1;
    float filter_y2;
    AudioSnapshot snapshot;
} AudioEngine;

void audio_init(AudioEngine* engine, float sample_rate);
void audio_update(AudioEngine* engine, float dt);
void audio_get_snapshot(AudioEngine* engine, AudioSnapshot* snapshot);
float audio_generate_sample(AudioEngine* engine);
void audio_note_on(Oscillator* osc, float frequency, float amplitude);
void audio_note_off(Oscillator* osc);
void audio_set_filter(AudioEngine* engine, float cutoff, float resonance);
float audio_sine(float phase);
float audio_square(float phase);
float audio_sawtooth(float phase);
float audio_noise(void);
float audio_filter(AudioEngine* engine, float input, float cutoff, float resonance);
int audio_device_init(AudioEngine* engine);
void audio_device_start(AudioEngine* engine);
void audio_device_cleanup(AudioEngine* engine);

#endif