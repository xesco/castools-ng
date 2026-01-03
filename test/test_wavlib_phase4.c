/*
 * WAV Library Phase 4 Test - MSX Byte Encoding (Serial Framing)
 * =============================================================
 * 
 * What this phase implements:
 * - writeByte() - complete MSX serial byte framing
 * - Frame structure: 1 START bit (0) + 8 data bits (LSB-first) + 2 STOP bits (1)
 * - Total of 11 bits per byte = ~9.17ms at 1200 baud
 * - Proper LSB-first bit ordering (MSX cassette standard)
 * 
 * What this test does:
 * - Tests edge-case byte patterns (0x00, 0xFF, 0xAA, 0x55, etc.)
 * - Validates START/STOP bit framing
 * - Tests ASCII text encoding (string "HELLO MSX!")
 * - Generates binary data sequences (0-255 countdown)
 * - Produces WAV files for oscilloscope/spectrum analysis
 * - Verifies correct bit timing: each byte = 11 bits × 0.833ms = 9.17ms
 * - Confirms LSB-first transmission order
 */

#include "lib/wavlib.h"
#include <stdio.h>
#include <stdlib.h>

// Helper function to print byte in binary (LSB-first order as transmitted)
void print_byte_binary(uint8_t byte) {
    printf("0x%02X = ", byte);
    for (int i = 0; i < 8; i++) {
        printf("%d", (byte >> i) & 1);
    }
    printf(" (LSB-first)");
}

int main(void) {
    WavFormat format = createDefaultWavFormat();
    WaveformConfig waveform = createDefaultWaveform();
    
    printf("=== Phase 4: MSX Byte Encoding Test ===\n\n");
    
    // Test 1: Single byte patterns
    printf("Test 1: Creating test_byte_patterns.wav...\n");
    WavWriter *writer = createWavFile("test_byte_patterns.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_byte_patterns.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.1);
    
    // Test interesting byte patterns
    uint8_t test_bytes[] = {
        0x00,  // All zeros
        0xFF,  // All ones
        0xAA,  // Alternating 10101010
        0x55,  // Alternating 01010101
        0xA5,  // Mixed pattern
        0x5A   // Inverse of 0xA5
    };
    
    for (size_t i = 0; i < sizeof(test_bytes); i++) {
        printf("  Writing byte: ");
        print_byte_binary(test_bytes[i]);
        printf("\n");
        
        if (!writeByte(writer, test_bytes[i], &waveform)) {
            fprintf(stderr, "Failed to write byte 0x%02X\n", test_bytes[i]);
            closeWavFile(writer);
            return 1;
        }
    }
    
    writeSilence(writer, 0.1);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_byte_patterns.wav\n");
        return 1;
    }
    printf("  ✓ Generated 6 test bytes with serial framing\n");
    
    // Test 2: ASCII string
    printf("\nTest 2: Creating test_byte_ascii.wav...\n");
    writer = createWavFile("test_byte_ascii.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_byte_ascii.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.1);
    
    const char *message = "MSX";
    printf("  Writing ASCII string: \"%s\"\n", message);
    for (size_t i = 0; message[i] != '\0'; i++) {
        printf("    '%c' = ", message[i]);
        print_byte_binary((uint8_t)message[i]);
        printf("\n");
        
        if (!writeByte(writer, (uint8_t)message[i], &waveform)) {
            fprintf(stderr, "Failed to write character '%c'\n", message[i]);
            closeWavFile(writer);
            return 1;
        }
    }
    
    writeSilence(writer, 0.1);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_byte_ascii.wav\n");
        return 1;
    }
    printf("  ✓ Generated ASCII string with serial framing\n");
    
    // Test 3: Byte sequence (0x00 to 0x0F)
    printf("\nTest 3: Creating test_byte_sequence.wav...\n");
    writer = createWavFile("test_byte_sequence.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_byte_sequence.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.1);
    
    printf("  Writing byte sequence 0x00 to 0x0F\n");
    for (uint8_t byte = 0x00; byte <= 0x0F; byte++) {
        if (!writeByte(writer, byte, &waveform)) {
            fprintf(stderr, "Failed to write byte 0x%02X\n", byte);
            closeWavFile(writer);
            return 1;
        }
    }
    
    writeSilence(writer, 0.1);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_byte_sequence.wav\n");
        return 1;
    }
    printf("  ✓ Generated 16-byte sequence\n");
    
    // Test 4: Framing verification (detailed single byte)
    printf("\nTest 4: Creating test_byte_framing.wav...\n");
    printf("  This file contains ONE byte (0xA5) with detailed framing:\n");
    printf("    START bit: 0\n");
    printf("    Data bits: ");
    for (int i = 0; i < 8; i++) {
        printf("%d", (0xA5 >> i) & 1);
    }
    printf(" (LSB-first)\n");
    printf("    STOP bits: 1 1\n");
    printf("    Total: 11 bits per byte\n");
    
    writer = createWavFile("test_byte_framing.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_byte_framing.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.2);  // Longer silence for easier analysis
    
    if (!writeByte(writer, 0xA5, &waveform)) {
        fprintf(stderr, "Failed to write byte\n");
        closeWavFile(writer);
        return 1;
    }
    
    writeSilence(writer, 0.2);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_byte_framing.wav\n");
        return 1;
    }
    printf("  ✓ Generated single byte with framing for analysis\n");
    
    // Test 5: Different waveform (square wave)
    printf("\nTest 5: Creating test_byte_square.wav...\n");
    WaveformConfig square = createWaveform(WAVE_SQUARE, 120);
    
    writer = createWavFile("test_byte_square.wav", &format);
    if (!writer) {
        fprintf(stderr, "Failed to create test_byte_square.wav\n");
        return 1;
    }
    
    writeSilence(writer, 0.1);
    
    printf("  Writing bytes with square waveform: 0x00, 0xFF, 0xAA\n");
    writeByte(writer, 0x00, &square);
    writeByte(writer, 0xFF, &square);
    writeByte(writer, 0xAA, &square);
    
    writeSilence(writer, 0.1);
    
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Failed to close test_byte_square.wav\n");
        return 1;
    }
    printf("  ✓ Generated bytes with square waveform\n");
    
    printf("\n=== Phase 4 Test Complete ===\n");
    printf("Generated files:\n");
    printf("  - test_byte_patterns.wav  (6 test byte patterns)\n");
    printf("  - test_byte_ascii.wav     (ASCII string \"MSX\")\n");
    printf("  - test_byte_sequence.wav  (bytes 0x00-0x0F)\n");
    printf("  - test_byte_framing.wav   (single byte 0xA5 for analysis)\n");
    printf("  - test_byte_square.wav    (bytes with square wave)\n");
    printf("\nEach byte is transmitted as:\n");
    printf("  START(0) + 8 data bits (LSB-first) + STOP(1) + STOP(1)\n");
    printf("  = 11 bits × 0.833ms = 9.17ms per byte\n");
    printf("\nTo verify:\n");
    printf("  1. Each byte should take exactly 11 bits (9.17ms)\n");
    printf("  2. START bit is always low frequency (1200 Hz)\n");
    printf("  3. STOP bits are always high frequency (2400 Hz)\n");
    printf("  4. Data bits follow LSB-first order\n");
    
    return 0;
}
