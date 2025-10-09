#include "sync_system.h"
#include <string.h>
#include <math.h>

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

void sync_init(RocketSync* sync) {
    memset(sync, 0, sizeof(RocketSync));
    sync->current.time = 0.0f;
    sync->current.beat = 0.0f;
    sync->current.bar = 0;
    sync->current.pattern = 0;
    sync->current.row = 0;
    sync->current.intensity = 0.0f;
    sync->current.bass = 0.0f;
    sync->current.mid = 0.0f;
    sync->current.high = 0.0f;
    sync->current.kick = false;
    sync->current.snare = false;
    sync->current.hihat = false;
}

void sync_update(RocketSync* sync, AudioEngine* audio, float dt) {
    sync->previous = sync->current;
    sync->current.time += dt;
    
    float bpm = 140.0f;
    float beats_per_second = bpm / 60.0f;
    sync->current.beat = sync->current.time * beats_per_second;
    sync->current.bar = (int)(sync->current.beat / 4.0f);
    
    float row_duration = 60.0f / (bpm * 4.0f);
    int total_rows = (int)(sync->current.time / row_duration);
    sync->current.row = total_rows % 64;
    sync->current.pattern = (total_rows / 64) % 8;
    
    int scene = (int)(sync->current.time / 12.0f) % 5;
    float scene_progress = fmodf(sync->current.time, 12.0f) / 12.0f;
    
    sync->current.intensity = 0.5f + scene_progress * 0.5f;
    
    AudioSnapshot snapshot = {0};
    if (audio) {
        audio_get_snapshot(audio, &snapshot);
    }
    
    if (audio && (snapshot.bass_energy > 0.001f || snapshot.mid_energy > 0.001f || snapshot.high_energy > 0.001f)) {
        float bass_scale = 8.0f;
        float mid_scale = 5.0f;
        float high_scale = 3.0f;
        
        float target_bass = snapshot.bass_energy * bass_scale;
        float target_mid = snapshot.mid_energy * mid_scale;
        float target_high = snapshot.high_energy * high_scale;
        
        float bass_smooth = 0.35f;
        float mid_smooth = 0.45f;
        float high_smooth = 0.5f;
        
        sync->current.bass = lerp(sync->previous.bass, target_bass, bass_smooth);
        sync->current.mid = lerp(sync->previous.mid, target_mid, mid_smooth);
        sync->current.high = lerp(sync->previous.high, target_high, high_smooth);
        
        if (sync->current.bass > 1.0f) sync->current.bass = 1.0f;
        if (sync->current.mid > 1.0f) sync->current.mid = 1.0f;
        if (sync->current.high > 1.0f) sync->current.high = 1.0f;
    } else {
        sync->current.bass = (scene >= 1) ? 0.6f : 0.3f;
        sync->current.mid = (scene >= 2) ? 0.5f : 0.2f;
        sync->current.high = (scene >= 3) ? 0.7f : 0.3f;
    }
    
    int prev_row = sync->previous.row;
    bool timing_kick = (sync->current.row % 4 == 0) && (prev_row % 4 != 0);
    bool timing_snare = (sync->current.row % 8 == 4) && (prev_row % 8 != 4);
    
    bool audio_kick = audio && (snapshot.bass_energy > 0.5f) && (snapshot.bass_energy > sync->previous.bass * 1.5f);
    bool audio_snare = audio && (snapshot.mid_energy > 0.4f) && (snapshot.mid_energy > sync->previous.mid * 1.3f);
    
    sync->current.kick = timing_kick || audio_kick;
    sync->current.snare = timing_snare || audio_snare;
    sync->current.hihat = (sync->current.row % 2 == 1);
    
    if (sync->transition_active) {
        sync->transition_time -= dt;
        if (sync->transition_time <= 0.0f) {
            sync->transition_active = false;
            sync->transition_time = 0.0f;
        }
    }
}

float sync_get_value(RocketSync* sync, const char* track_name) {
    // Simple track value mapping based on common demoscene tracks
    if (strcmp(track_name, "intensity") == 0) {
        if (sync->transition_active) {
            float t = 1.0f - (sync->transition_time / 2.0f);
            return lerp(sync->previous.intensity, sync->current.intensity, t);
        }
        return sync->current.intensity;
    }
    
    if (strcmp(track_name, "bass") == 0) {
        if (sync->transition_active) {
            float t = 1.0f - (sync->transition_time / 2.0f);
            return lerp(sync->previous.bass, sync->current.bass, t);
        }
        return sync->current.bass;
    }
    
    if (strcmp(track_name, "mid") == 0) {
        if (sync->transition_active) {
            float t = 1.0f - (sync->transition_time / 2.0f);
            return lerp(sync->previous.mid, sync->current.mid, t);
        }
        return sync->current.mid;
    }
    
    if (strcmp(track_name, "high") == 0) {
        if (sync->transition_active) {
            float t = 1.0f - (sync->transition_time / 2.0f);
            return lerp(sync->previous.high, sync->current.high, t);
        }
        return sync->current.high;
    }
    
    if (strcmp(track_name, "beat") == 0) {
        return fmodf(sync->current.beat, 1.0f);
    }
    
    if (strcmp(track_name, "bar") == 0) {
        return (float)sync->current.bar;
    }
    
    if (strcmp(track_name, "pattern") == 0) {
        return (float)sync->current.pattern;
    }
    
    if (strcmp(track_name, "time") == 0) {
        return sync->current.time;
    }
    
    // Default values for unknown tracks
    if (strstr(track_name, "rotate") != NULL) {
        return sync->current.time * 0.5f;
    }
    
    if (strstr(track_name, "pulse") != NULL) {
        return sinf(sync->current.time * 2.0f) * 0.5f + 0.5f;
    }
    
    if (strstr(track_name, "wave") != NULL) {
        return sinf(sync->current.beat * 0.5f) * 0.5f + 0.5f;
    }
    
    return 0.0f;
}

bool sync_get_trigger(RocketSync* sync, const char* trigger_name) {
    if (strcmp(trigger_name, "kick") == 0) {
        return sync->current.kick && !sync->previous.kick;
    }
    
    if (strcmp(trigger_name, "snare") == 0) {
        return sync->current.snare && !sync->previous.snare;
    }
    
    if (strcmp(trigger_name, "hihat") == 0) {
        return sync->current.hihat && !sync->previous.hihat;
    }
    
    if (strcmp(trigger_name, "beat") == 0) {
        return ((int)sync->current.beat != (int)sync->previous.beat);
    }
    
    if (strcmp(trigger_name, "bar") == 0) {
        return (sync->current.bar != sync->previous.bar);
    }
    
    if (strcmp(trigger_name, "pattern") == 0) {
        return (sync->current.pattern != sync->previous.pattern);
    }
    
    return false;
}

void sync_set_transition(RocketSync* sync, float duration) {
    sync->transition_active = true;
    sync->transition_time = duration;
}