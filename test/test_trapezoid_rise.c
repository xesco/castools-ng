/*
 * Trapezoid Rise Time Test - Configurable Waveform Slopes
 * ========================================================
 * 
 * This test generates trapezoid waves with different rise times to compare:
 * 1. test_trap_rise5.wav  - Sharp 5% rise time (steep edges)
 * 2. test_trap_rise10.wav - Balanced 10% rise time (default)
 * 3. test_trap_rise15.wav - Gentle 15% rise time
 * 4. test_trap_rise20.wav - Very gentle 20% rise time
 * 
 * Purpose: Verify configurable rise time works correctly and demonstrate
 *          the range from sharp (better signal discrimination) to gentle
 *          (better for worn tape decks and audio connections).
 */

#include "../lib/wavlib.h"
#include "test_utils.h"
#include <stdio.h>

static bool create_trapezoid_test(const char *filename, uint8_t rise_percent) {
    WavFormat fmt = createDefaultWavFormat();
    
    WaveformConfig config = createWaveform(WAVE_TRAPEZOID, 120);
    config.baud_rate = 2400;
    config.trapezoid_rise_percent = rise_percent;
    
    WavWriter *wav = createWavFile(filename, &fmt);
    if (!wav) {
        fprintf(stderr, "Failed to create %s\n", filename);
        return false;
    }
    
    // Write 50 pulses at 2400 Hz
    for (int i = 0; i < 50; i++) {
        if (!writePulse(wav, 2400, &config)) {
            fprintf(stderr, "Failed to write pulse!\n");
            closeWavFile(wav);
            return false;
        }
    }
    
    closeWavFile(wav);
    return true;
}

int main(void) {
    printf("Trapezoid Rise Time Test\n");
    printf("=========================\n\n");
    
    printf("Generating trapezoid waves with different rise times...\n\n");
    
    // Test 1: Sharp edges (5%)
    printf("1. Creating 5%% rise time (sharp edges)...\n");
    if (!create_trapezoid_test("test_trap_rise5.wav", 5)) {
        return 1;
    }
    printf("   ✓ test_trap_rise5.wav - Steep slopes, better signal discrimination\n\n");
    
    // Test 2: Balanced (10% - default)
    printf("2. Creating 10%% rise time (balanced - default)...\n");
    if (!create_trapezoid_test("test_trap_rise10.wav", 10)) {
        return 1;
    }
    printf("   ✓ test_trap_rise10.wav - Default setting, good all-around\n\n");
    
    // Test 3: Gentle (15%)
    printf("3. Creating 15%% rise time (gentle)...\n");
    if (!create_trapezoid_test("test_trap_rise15.wav", 15)) {
        return 1;
    }
    printf("   ✓ test_trap_rise15.wav - Gentler slopes, easier on hardware\n\n");
    
    // Test 4: Very gentle (20%)
    printf("4. Creating 20%% rise time (very gentle)...\n");
    if (!create_trapezoid_test("test_trap_rise20.wav", 20)) {
        return 1;
    }
    printf("   ✓ test_trap_rise20.wav - Very gentle slopes, best for worn equipment\n\n");
    
    // Summary
    printf("========================================\n");
    printf("✓ All tests complete!\n\n");
    printf("Comparison in audio editor:\n");
    printf("  5%%:  Sharpest edges (closest to square wave)\n");
    printf("  10%%: Balanced (default, recommended)\n");
    printf("  15%%: Gentler slopes\n");
    printf("  20%%: Gentlest slopes (most tape-like)\n\n");
    printf("All should decode correctly on MSX.\n");
    printf("Choose based on your playback hardware quality.\n");
    
    return 0;
}
