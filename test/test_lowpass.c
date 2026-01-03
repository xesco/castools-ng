/*
 * Low-Pass Filter Test - Audio Quality Enhancement
 * ================================================
 * 
 * This test generates two WAV files to compare filtered vs unfiltered output:
 * 1. test_square_raw.wav - Square wave without filter (harsh harmonics)
 * 2. test_square_filtered.wav - Square wave with 6000 Hz low-pass filter (smooth)
 * 
 * Purpose: Verify the low-pass filter reduces high-frequency harmonics while
 *          preserving the fundamental signal for cleaner MSX playback.
 */

#include "../lib/wavlib.h"
#include <stdio.h>

int main(void) {
    printf("Low-Pass Filter Test\n");
    printf("====================\n\n");
    
    WavFormat fmt = createDefaultWavFormat();
    
    // Test 1: Square wave WITHOUT filter (harsh)
    printf("Test 1: Creating square wave WITHOUT filter...\n");
    WaveformConfig config_raw = createWaveform(WAVE_SQUARE, 120);
    config_raw.baud_rate = 2400;
    config_raw.enable_lowpass = false;  // Filter disabled
    
    WavWriter *wav_raw = createWavFile("test_square_raw.wav", &fmt);
    if (!wav_raw) {
        fprintf(stderr, "Failed to create raw WAV file!\n");
        return 1;
    }
    
    // Write 100 pulses at 2400 Hz
    for (int i = 0; i < 100; i++) {
        if (!writePulse(wav_raw, 2400, &config_raw)) {
            fprintf(stderr, "Failed to write pulse!\n");
            closeWavFile(wav_raw);
            return 1;
        }
    }
    
    closeWavFile(wav_raw);
    printf("  Created: test_square_raw.wav\n");
    printf("  Filter: DISABLED\n");
    printf("  Harmonics: Full spectrum (harsh)\n\n");
    
    // Test 2: Square wave WITH filter (smooth)
    printf("Test 2: Creating square wave WITH low-pass filter...\n");
    WaveformConfig config_filtered = createWaveform(WAVE_SQUARE, 120);
    config_filtered.baud_rate = 2400;
    config_filtered.enable_lowpass = true;      // Filter enabled
    config_filtered.lowpass_cutoff_hz = 6000;   // 6 kHz cutoff
    
    WavWriter *wav_filtered = createWavFile("test_square_filtered.wav", &fmt);
    if (!wav_filtered) {
        fprintf(stderr, "Failed to create filtered WAV file!\n");
        return 1;
    }
    
    // Write 100 pulses at 2400 Hz
    for (int i = 0; i < 100; i++) {
        if (!writePulse(wav_filtered, 2400, &config_filtered)) {
            fprintf(stderr, "Failed to write pulse!\n");
            closeWavFile(wav_filtered);
            return 1;
        }
    }
    
    closeWavFile(wav_filtered);
    printf("  Created: test_square_filtered.wav\n");
    printf("  Filter: ENABLED (6000 Hz cutoff)\n");
    printf("  Harmonics: Reduced (smooth)\n\n");
    
    // Summary
    printf("========================================\n");
    printf("✓ Test complete!\n\n");
    printf("Compare the files in an audio editor:\n");
    printf("  - test_square_raw.wav: Sharp edges, full harmonics\n");
    printf("  - test_square_filtered.wav: Smoother, reduced harmonics\n\n");
    printf("Both should decode correctly on MSX, but the filtered\n");
    printf("version will have cleaner playback from computer audio.\n");
    
    return 0;
}
