#include "test_utils.h"
#include <stdio.h>

// Silence/leader duration constants
#define SILENCE_LONG_HEADER  2.0f
#define SILENCE_SHORT_HEADER 1.0f

WaveformConfig createWaveform(WaveformType type, uint8_t amplitude) {
    WaveformConfig config = {
        .type = type,
        .amplitude = amplitude,
        .baud_rate = 1200,  // Standard MSX baud rate
        .sample_rate = 43200,  // Default MSX sample rate
        .custom_samples = NULL,
        .custom_length = 0,
        .trapezoid_rise_percent = 10,  // 10% rise/fall time for trapezoid
        .long_silence = SILENCE_LONG_HEADER,   // 2s before file header
        .short_silence = SILENCE_SHORT_HEADER, // 1s before data blocks
        .enable_lowpass = false,     // Disabled by default
        .lowpass_cutoff_hz = 6000    // Sensible default
    };
    return config;
}

bool setTrapezoidRiseTime(WaveformConfig *config, uint8_t rise_percent) {
    if (!config) {
        return false;
    }
    
    if (config->type != WAVE_TRAPEZOID) {
        fprintf(stderr, "Warning: Rise time only applies to trapezoid waveform\n");
        return false;
    }
    
    // Clamp to valid range (1-50%)
    if (rise_percent < 1) rise_percent = 1;
    if (rise_percent > 50) rise_percent = 50;
    
    config->trapezoid_rise_percent = rise_percent;
    return true;
}
