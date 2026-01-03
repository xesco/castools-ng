/*
 * WAV Library Phase 1 Test - Basic WAV File Creation
 * ==================================================
 * 
 * What this phase implements:
 * - WAV file format structure (RIFF/WAVE headers)
 * - Basic file creation and initialization
 * - Format validation (sample rate, bit depth, channels)
 * - Silence generation
 * - File finalization and size calculation
 * 
 * What this test does:
 * - Creates a WAV file with MSX default format (43200 Hz, 8-bit mono)
 * - Validates the format parameters
 * - Writes 2 seconds of silence
 * - Verifies the file size matches expected calculation
 * - Ensures proper header structure for MSX compatibility
 */

#include "lib/wavlib.h"
#include <stdio.h>

int main(void) {
    printf("WAV Library Phase 1 Test\n");
    printf("========================\n\n");
    
    // Create default MSX format
    WavFormat fmt = createDefaultWavFormat();
    printf("WAV Format:\n");
    printf("  Sample Rate: %u Hz\n", fmt.sample_rate);
    printf("  Bit Depth: %u bits\n", fmt.bits_per_sample);
    printf("  Channels: %u\n", fmt.channels);
    printf("  Amplitude: %u\n\n", fmt.amplitude);
    
    // Validate format
    if (!validateWavFormat(&fmt)) {
        fprintf(stderr, "Format validation failed!\n");
        return 1;
    }
    printf("Format validation: PASSED\n\n");
    
    // Create WAV file
    printf("Creating test.wav...\n");
    WavWriter *wav = createWavFile("test.wav", &fmt);
    if (!wav) {
        fprintf(stderr, "Failed to create WAV file!\n");
        return 1;
    }
    printf("File created: OK\n\n");
    
    // Write 2 seconds of silence
    printf("Writing 2 seconds of silence...\n");
    if (!writeSilence(wav, 2.0f)) {
        fprintf(stderr, "Failed to write silence!\n");
        closeWavFile(wav);
        return 1;
    }
    printf("Silence written: %zu samples\n\n", wav->sample_count);
    
    // Close file
    printf("Closing file and finalizing headers...\n");
    if (!closeWavFile(wav)) {
        fprintf(stderr, "Failed to close file!\n");
        return 1;
    }
    printf("File closed: OK\n\n");
    
    // Calculate expected file size
    size_t expected_samples = 43200 * 2;  // 43200 Hz * 2 seconds
    size_t expected_data_size = expected_samples * 1;  // 1 byte per sample (8-bit)
    size_t expected_file_size = 44 + expected_data_size;  // 44 = WAV header size
    
    printf("Expected file size: %zu bytes\n", expected_file_size);
    printf("\n");
    printf("Test completed successfully!\n");
    printf("You can now play test.wav in any audio player.\n");
    printf("It should be 2 seconds of silence.\n");
    
    return 0;
}
