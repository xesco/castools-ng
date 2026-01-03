/*
 * WAV Library Phase 6 Test - Complete CAS to WAV Conversion
 * =========================================================
 * 
 * What this phase implements:
 * - convertCasToWav() - complete CAS file to MSX tape audio conversion
 * - Multi-file CAS container support
 * - File header blocks (type marker + filename)
 * - Data block headers (load/end/exec addresses for BINARY/BASIC)
 * - Proper sync sequences (8000 for file headers, 2000 for data blocks)
 * - Silence padding between blocks (2s for headers, 1s for data)
 * - ASCII padding bytes (0x1A EOF markers)
 * - BASIC padding bytes (after end-of-program marker)
 * - BINARY data (padding included in data section)
 * 
 * What this test does:
 * - Accepts CAS file as command-line argument
 * - Converts complete CAS file to MSX-compatible WAV audio
 * - Tests all file types (ASCII, BASIC, BINARY, custom blocks)
 * - Produces WAV files loadable in openMSX emulator or real MSX hardware
 * - Verifies complete tape protocol: silence → sync → header → data → repeat
 */

#include "lib/wavlib.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("=== Phase 6: CAS to WAV Conversion Test ===\n\n");
    
    // Check if CAS file argument is provided
    const char *cas_file = (argc > 1) ? argv[1] : "basic_with_marker.cas";
    const char *wav_file = (argc > 2) ? argv[2] : "output.wav";
    
    printf("Input:  %s\n", cas_file);
    printf("Output: %s\n\n", wav_file);
    
    // Create default waveform configuration
    WaveformConfig waveform = createDefaultWaveform();
    
    // Convert CAS to WAV
    if (!convertCasToWav(cas_file, wav_file, &waveform)) {
        fprintf(stderr, "\n❌ Conversion failed!\n");
        return 1;
    }
    
    printf("\n=== Conversion Summary ===\n");
    printf("Generated: %s\n", wav_file);
    printf("Format: WAVE audio, 8-bit mono, 43200 Hz\n");
    printf("Encoding: MSX cassette tape format (1200 baud FSK)\n");
    printf("\nTo verify:\n");
    printf("  1. Play the file - should hear high-pitched tones\n");
    printf("  2. Load in MSX emulator or real MSX hardware\n");
    printf("  3. Check file command: file %s\n", wav_file);
    
    return 0;
}
