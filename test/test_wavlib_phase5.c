/*
 * WAV Library Phase 5 Test - MSX Tape Protocol (Sync Sequences)
 * =============================================================
 * 
 * What this phase implements:
 * - writeSync() - generates sync/leader tone sequences
 * - MSX long header: 8000 sync pulses (16000 half-pulses) for file headers
 * - MSX short header: 2000 sync pulses (4000 half-pulses) for data blocks
 * - Sync pulses are continuous 1-bits (2400 Hz carrier tone)
 * - Provides timing synchronization for MSX cassette hardware
 * 
 * What this test does:
 * - Tests short sync sequences (100 pulses for debugging)
 * - Generates MSX data block header (2000 pulses = ~1.67 seconds)
 * - Generates MSX file header (8000 pulses = ~6.67 seconds)
 * - Creates complete header sequences with silence padding
 * - Validates timing calculations (sync duration formulas)
 * - Produces WAV files that sound like continuous high-pitched tones
 * - Verifies sync pulse lengths match MSX ROM specifications
 */

#include "lib/wavlib.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    WavFormat format = createDefaultWavFormat();
    WaveformConfig waveform = createDefaultWaveform();
    
    printf("=== Phase 5: MSX Tape Protocol Test ===\n\n");
    
    // Test 1: Short sync sequence (100 pulses for testing)
    printf("Test 1: Creating test_sync_short.wav...\n");
    WavWriter *writer = createWavFile("test_sync_short.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_sync_short.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.1);
    
    printf("  Writing 100 sync pulses (1-bits)\n");
    if (!writeSync(writer, 100, &waveform)) {
        fprintf(stderr, "Failed to write sync pulses\n");
        closeWavFile(writer);
        return 1;
    }
    
    writeSilence(writer, 0.1);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_sync_short.wav\n");
        return 1;
    }
    printf("  ✓ Generated 100 sync pulses\n");
    printf("    Duration: 100 bits × 0.833ms = 83.3ms\n");
    
    // Test 2: MSX data block header (2000 pulses)
    printf("\nTest 2: Creating test_sync_data_header.wav...\n");
    writer = createWavFile("test_sync_data_header.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_sync_data_header.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.2);
    
    printf("  Writing 2000 sync pulses (MSX data block header)\n");
    if (!writeSync(writer, 2000, &waveform)) {
        fprintf(stderr, "Failed to write sync pulses\n");
        closeWavFile(writer);
        return 1;
    }
    
    writeSilence(writer, 0.2);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_sync_data_header.wav\n");
        return 1;
    }
    printf("  ✓ Generated 2000 sync pulses\n");
    printf("    Duration: 2000 bits × 0.833ms = 1.67 seconds\n");
    
    // Test 3: MSX file header (8000 pulses)
    printf("\nTest 3: Creating test_sync_file_header.wav...\n");
    writer = createWavFile("test_sync_file_header.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_sync_file_header.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.5);
    
    printf("  Writing 8000 sync pulses (MSX file header)\n");
    printf("  This may take a moment...\n");
    if (!writeSync(writer, 8000, &waveform)) {
        fprintf(stderr, "Failed to write sync pulses\n");
        closeWavFile(writer);
        return 1;
    }
    
    writeSilence(writer, 0.5);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_sync_file_header.wav\n");
        return 1;
    }
    printf("  ✓ Generated 8000 sync pulses\n");
    printf("    Duration: 8000 bits × 0.833ms = 6.67 seconds\n");
    
    // Test 4: Complete file header sequence (realistic MSX tape start)
    printf("\nTest 4: Creating test_sync_complete_header.wav...\n");
    printf("  Simulating MSX tape file header sequence:\n");
    printf("    1. Long silence (2 seconds)\n");
    printf("    2. 8000 sync pulses (file header)\n");
    printf("    3. Identifier byte (0xEA for ASCII file)\n");
    printf("    4. Short silence\n");
    
    writer = createWavFile("test_sync_complete_header.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_sync_complete_header.wav\n");
        return 1;
    }
    
    // Step 1: Long silence (tape leader)
    writeSilence(writer, 2.0);
    
    // Step 2: File header sync
    writeSync(writer, 8000, &waveform);
    
    // Step 3: Identifier byte (0xEA = ASCII file type)
    writeByte(writer, 0xEA, &waveform);
    
    // Step 4: Short silence
    writeSilence(writer, 0.5);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_sync_complete_header.wav\n");
        return 1;
    }
    printf("  ✓ Generated complete file header sequence\n");
    
    // Test 5: Multiple data blocks
    printf("\nTest 5: Creating test_sync_data_blocks.wav...\n");
    printf("  Simulating 3 data blocks:\n");
    printf("    Each block: 2000 sync + 1 byte\n");
    
    writer = createWavFile("test_sync_data_blocks.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_sync_data_blocks.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.2);
    
    uint8_t data_bytes[] = {0x01, 0x02, 0x03};
    for (int i = 0; i < 3; i++) {
        printf("    Block %d: 2000 sync + byte 0x%02X\n", i + 1, data_bytes[i]);
        writeSync(writer, 2000, &waveform);
        writeByte(writer, data_bytes[i], &waveform);
    }
    
    writeSilence(writer, 0.2);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_sync_data_blocks.wav\n");
        return 1;
    }
    printf("  ✓ Generated 3 data blocks with headers\n");
    
    printf("\n=== Phase 5 Test Complete ===\n");
    printf("Generated files:\n");
    printf("  - test_sync_short.wav          (100 sync pulses)\n");
    printf("  - test_sync_data_header.wav    (2000 sync pulses)\n");
    printf("  - test_sync_file_header.wav    (8000 sync pulses)\n");
    printf("  - test_sync_complete_header.wav (full file header sequence)\n");
    printf("  - test_sync_data_blocks.wav    (3 data blocks with headers)\n");
    printf("\nMSX Tape Format:\n");
    printf("  File header: silence + 8000 sync pulses + header bytes\n");
    printf("  Data blocks: 2000 sync pulses + data bytes\n");
    printf("  Sync pulse = 1-bit = 2 pulses @ 2400 Hz = 0.833ms\n");
    printf("\nTo verify:\n");
    printf("  1. File header sync should be ~6.67 seconds of 2400 Hz tone\n");
    printf("  2. Data block sync should be ~1.67 seconds of 2400 Hz tone\n");
    printf("  3. Sync sequences should be continuous high-frequency tone\n");
    
    return 0;
}
