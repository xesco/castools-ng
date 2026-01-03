/*
 * WAV Library Phase 2 Test - Waveform Generation
 * ==============================================
 * 
 * What this phase implements:
 * - Pulse generation at specific frequencies
 * - Multiple waveform types (sine, square, triangle, trapezoid)
 * - Waveform configuration system
 * - Amplitude control
 * - Frequency calculation for MSX tape signals
 * 
 * What this test does:
 * - Generates test files for all waveform types
 * - Tests MSX carrier frequencies (1200 Hz for 0-bit, 2400 Hz for 1-bit)
 * - Creates 0.5 second samples of each waveform/frequency combination
 * - Produces 8 WAV files for audio verification
 * - Validates that different waveforms produce audibly different tones
 */

#include "lib/wavlib.h"
#include <stdio.h>

int main(void) {
    printf("WAV Library Phase 2 Test - Waveform Generation\n");
    printf("===============================================\n\n");
    
    // Test parameters
    const char *waveform_names[] = {"sine", "square", "triangle", "trapezoid"};
    WaveformType waveform_types[] = {WAVE_SINE, WAVE_SQUARE, WAVE_TRIANGLE, WAVE_TRAPEZOID};
    uint16_t test_frequencies[] = {1200, 2400};  // MSX frequencies
    
    for (int w = 0; w < 4; w++) {
        for (int f = 0; f < 2; f++) {
            char filename[64];
            snprintf(filename, sizeof(filename), "test_%s_%dHz.wav", 
                    waveform_names[w], test_frequencies[f]);
            
            printf("Generating %s...\n", filename);
            
            // Create WAV file
            WavFormat fmt = createDefaultWavFormat();
            WavWriter *wav = createWavFile(filename, &fmt);
            if (!wav) {
                fprintf(stderr, "Failed to create %s\n", filename);
                return 1;
            }
            
            // Create waveform config
            WaveformConfig config = createWaveform(waveform_types[w], 120);
            
            // Generate 0.5 seconds of tone (write multiple pulses)
            float duration = 0.5f;
            size_t num_pulses = (size_t)(test_frequencies[f] * duration);
            
            for (size_t i = 0; i < num_pulses; i++) {
                if (!writePulse(wav, test_frequencies[f], &config)) {
                    fprintf(stderr, "Failed to write pulse!\n");
                    closeWavFile(wav);
                    return 1;
                }
            }
            
            printf("  Wrote %zu pulses (%zu samples)\n", num_pulses, wav->sample_count);
            
            // Close file
            closeWavFile(wav);
            printf("  Saved: %s\n\n", filename);
        }
    }
    
    printf("Phase 2 test completed!\n\n");
    printf("Generated files:\n");
    for (int w = 0; w < 4; w++) {
        for (int f = 0; f < 2; f++) {
            printf("  - test_%s_%dHz.wav (0.5 sec, %d Hz tone)\n", 
                   waveform_names[w], test_frequencies[f], test_frequencies[f]);
        }
    }
    printf("\nYou can open these in an audio editor (like Audacity) to:\n");
    printf("  1. Verify the waveform shapes\n");
    printf("  2. Confirm the frequencies (1200 Hz and 2400 Hz)\n");
    printf("  3. Listen to the MSX cassette tones!\n");
    
    return 0;
}
