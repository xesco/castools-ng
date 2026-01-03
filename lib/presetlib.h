#ifndef PRESETLIB_H
#define PRESETLIB_H

#include <stdbool.h>
#include <stdint.h>
#include "wavlib.h"

// =============================================================================
// Audio Profile Presets
// =============================================================================

// Profile metadata
typedef struct {
    const char *name;              // Profile name (e.g., "default")
    const char *short_desc;        // One-line description
    const char *use_case;          // Detailed use case description
    const char *category;          // Category (e.g., "General Purpose")
    
    // Audio configuration
    WaveformType waveform;
    uint16_t baud_rate;
    uint8_t amplitude;
    uint8_t trapezoid_rise_percent;
    float long_silence;
    float short_silence;
    bool enable_lowpass;
    uint16_t lowpass_cutoff_hz;
    
    const char *rationale;         // Why these settings work
} AudioProfile;

// =============================================================================
// Profile Management Functions
// =============================================================================

// Get total number of available profiles
size_t getProfileCount(void);

// Get profile by index (for iteration)
const AudioProfile* getProfileByIndex(size_t index);

// Find profile by name (case-insensitive)
// Returns NULL if not found
const AudioProfile* findProfile(const char *name);

// Apply profile to WaveformConfig
void applyProfile(WaveformConfig *config, const AudioProfile *profile);

#endif // PRESETLIB_H
