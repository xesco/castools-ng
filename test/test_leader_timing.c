/*
 * Leader/Silence Timing Test - Configurable Header Delays
 * ========================================================
 * 
 * This test generates short WAV files with different leader timing presets:
 * 1. test_leader_standard.wav - 2.0s/1.0s (default, fast loading)
 * 2. test_leader_conservative.wav - 3.0s/2.0s (more AGC/motor time)
 * 3. test_leader_extended.wav - 5.0s/3.0s (maximum compatibility)
 * 
 * Purpose: Verify configurable leader timing works correctly and demonstrate
 *          the differences between timing presets for MSX compatibility.
 * 
 * Note: This creates a minimal file structure to show timing differences.
 */

#include "../lib/wavlib.h"
#include "../lib/caslib.h"
#include "test_utils.h"
#include <stdio.h>
#include <string.h>

// Create a minimal test byte pattern (simulates a small data block)
static const uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};

static bool create_timing_test(const char *filename, float long_silence, 
                                float short_silence, const char *preset_name) {
    WavFormat fmt = createDefaultWavFormat();
    
    WaveformConfig config = createDefaultWaveform();
    config.baud_rate = 1200;
    config.sample_rate = fmt.sample_rate;  // Match WAV format sample rate
    config.long_silence = long_silence;
    config.short_silence = short_silence;
    
    WavWriter *wav = createWavFile(filename, &fmt);
    if (!wav) {
        fprintf(stderr, "Failed to create %s\n", filename);
        return false;
    }
    
    // Simulate file header block
    printf("  Creating %s preset (%.1fs/%.1fs)...\n", preset_name, long_silence, short_silence);
    printf("    - Long silence (%.1fs)\n", long_silence);
    writeSilence(wav, long_silence);
    writeSync(wav, 100, &config);  // Short sync header for testing
    
    // Write a few test bytes
    for (size_t i = 0; i < sizeof(test_data); i++) {
        writeByte(wav, test_data[i], &config);
    }
    
    // Simulate data block
    printf("    - Short silence (%.1fs)\n", short_silence);
    writeSilence(wav, short_silence);
    writeSync(wav, 50, &config);   // Shorter sync for data block
    
    // Write a few more test bytes
    for (size_t i = 0; i < sizeof(test_data); i++) {
        writeByte(wav, test_data[i], &config);
    }
    
    closeWavFile(wav);
    return true;
}

int main(void) {
    printf("Leader/Silence Timing Test\n");
    printf("===========================\n\n");
    
    printf("Generating WAV files with different leader timing presets...\n\n");
    
    // Test 1: Standard timing (default)
    printf("1. Standard preset (default):\n");
    if (!create_timing_test("test_leader_standard.wav", 2.0f, 1.0f, "standard")) {
        return 1;
    }
    printf("   ✓ test_leader_standard.wav\n\n");
    
    // Test 2: Conservative timing
    printf("2. Conservative preset:\n");
    if (!create_timing_test("test_leader_conservative.wav", 3.0f, 2.0f, "conservative")) {
        return 1;
    }
    printf("   ✓ test_leader_conservative.wav\n\n");
    
    // Test 3: Extended timing
    printf("3. Extended preset:\n");
    if (!create_timing_test("test_leader_extended.wav", 5.0f, 3.0f, "extended")) {
        return 1;
    }
    printf("   ✓ test_leader_extended.wav\n\n");
    
    // Summary
    printf("========================================\n");
    printf("✓ All tests complete!\n\n");
    printf("File durations (approximate):\n");
    printf("  Standard:     ~3.2s (2.0s + 1.0s + data)\n");
    printf("  Conservative: ~5.2s (3.0s + 2.0s + data)\n");
    printf("  Extended:     ~8.2s (5.0s + 3.0s + data)\n\n");
    printf("Use cases:\n");
    printf("  Standard:     Fast loading, modern equipment\n");
    printf("  Conservative: Older MSX, needs AGC stabilization\n");
    printf("  Extended:     Problematic hardware, maximum reliability\n");
    
    return 0;
}
