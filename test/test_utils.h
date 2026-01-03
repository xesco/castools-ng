#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../lib/wavlib.h"

// Test utility functions for creating waveform configurations

// Create custom waveform config with specific type
WaveformConfig createWaveform(WaveformType type, uint8_t amplitude);

// Set trapezoid rise time (only applies to WAVE_TRAPEZOID)
// rise_percent: Rise/fall time as percentage of cycle (5, 10, 15, 20)
//               Valid range: 1-50 (values outside are clamped)
// Returns false if waveform type is not trapezoid
bool setTrapezoidRiseTime(WaveformConfig *config, uint8_t rise_percent);

#endif // TEST_UTILS_H
