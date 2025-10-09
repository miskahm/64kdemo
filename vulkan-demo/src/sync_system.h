#ifndef SYNC_SYSTEM_H
#define SYNC_SYSTEM_H

#include "audio_synthesis.h"
#include <stdbool.h>

typedef struct {
    float time;
    float beat;
    int bar;
    int pattern;
    int row;
    float intensity;
    float bass;
    float mid;
    float high;
    bool kick;
    bool snare;
    bool hihat;
} SyncData;

typedef struct {
    SyncData current;
    SyncData previous;
    float transition_time;
    bool transition_active;
} RocketSync;

void sync_init(RocketSync* sync);
void sync_update(RocketSync* sync, AudioEngine* audio, float dt);
float sync_get_value(RocketSync* sync, const char* track_name);
bool sync_get_trigger(RocketSync* sync, const char* trigger_name);
void sync_set_transition(RocketSync* sync, float duration);

#endif