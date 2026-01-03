#ifndef WAVLIB_H
#define WAVLIB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// =============================================================================
// WAV File Generation for CAS to WAV Conversion
// =============================================================================
//
// CONVERSION STRATEGY (CAS → WAV):
//
// 1. Parse CAS file structure (using caslib - already implemented)
//    - Scan for CAS HEADER delimiters (1F A6 DE BA CC 13 7D 74)
//    - Identify file header blocks vs data blocks
//    - Extract actual data bytes (strip CAS headers)
//
// 2. For each CAS HEADER found, determine what MSX tape sequence to generate:
//    - First CAS HEADER of a file (file header block):
//      → Long silence (2 seconds)
//      → Initial sync (8000 1-bits) = LONG_HEADER (16000 pulses @ 2400 Hz)
//      → Encode 16 bytes: type marker (10 bytes) + filename (6 bytes)
//    
//    - Subsequent CAS HEADERs (data blocks):
//      → Short silence (1 second)
//      → Block sync (2000 1-bits) = SHORT_HEADER (4000 pulses @ 2400 Hz)
//      → Encode all data bytes in the block
//
// 3. Encode each data byte using MSX serial framing (11 bits per byte):
//    - 1 START bit (0-bit = 1 pulse at 1200 Hz)
//    - 8 DATA bits (LSB first: 0-bit = 1200 Hz, 1-bit = 2400 Hz)
//    - 2 STOP bits (1-bits = 2400 Hz each)
//
// 4. Write final WAV file with proper headers
//
// IMPLEMENTATION APPROACH:
// - Build primitives bottom-up (pulse → bit → byte → sync → file)
// - Test each layer independently before moving up
// - Validate output at each step (check waveforms, timing, byte encoding)
//
// =============================================================================

// =============================================================================
// Waveform Types
// =============================================================================

typedef enum {
    WAVE_SINE,       // Smooth sine wave (most natural, default)
    WAVE_SQUARE,     // Sharp transitions (digital ideal)
    WAVE_TRIANGLE,   // Linear ramps (symmetric)
    WAVE_TRAPEZOID,  // Square with sloped edges (realistic cassette)
    WAVE_CUSTOM      // User-provided sample data
} WaveformType;

// =============================================================================
// WAV File Format Configuration
// =============================================================================

// WAV file format parameters
typedef struct {
    uint32_t sample_rate;      // Samples per second (e.g., 43200 Hz)
    uint16_t bits_per_sample;  // 8 or 16 bits
    uint16_t channels;         // 1 (mono) or 2 (stereo)
    uint8_t amplitude;         // Peak amplitude (0-127 for 8-bit, 0-32767 for 16-bit)
} WavFormat;

// Waveform generation configuration
typedef struct {
    WaveformType type;              // Type of waveform to generate
    uint8_t amplitude;              // Peak amplitude for this waveform
    uint16_t baud_rate;             // MSX baud rate: 1200 (standard) or 2400 (turbo)
    const uint8_t *custom_samples;  // For WAVE_CUSTOM: pre-calculated samples
    size_t custom_length;           // Number of samples in custom waveform
} WaveformConfig;

// WAV file writer context (opaque to user)
typedef struct {
    FILE *file;
    WavFormat format;
    size_t sample_count;
    long data_chunk_pos;
} WavWriter;

// =============================================================================
// Factory Functions - Easy Defaults
// =============================================================================

// Create default MSX WAV format (43200 Hz, 8-bit, mono, amplitude 120)
WavFormat createDefaultWavFormat(void);

// Create default waveform config (sine wave with default amplitude)
WaveformConfig createDefaultWaveform(void);

// Create custom waveform config with specific type
WaveformConfig createWaveform(WaveformType type, uint8_t amplitude);

// =============================================================================
// Validation
// =============================================================================

// Validate WAV format settings
// Returns true if valid, false otherwise
bool validateWavFormat(const WavFormat *format);

// =============================================================================
// WAV File Management
// =============================================================================

// Create and open a new WAV file with specified format
// Returns NULL on error
WavWriter* createWavFile(const char *filename, const WavFormat *format);

// Close WAV file and finalize headers
// Returns false on error
bool closeWavFile(WavWriter *writer);

// Write raw samples directly to WAV file
// Returns false on error
bool writeSamples(WavWriter *writer, const uint8_t *samples, size_t count);

// =============================================================================
// Pulse Generation - Low Level
// =============================================================================

// Generate one complete pulse (wave cycle) at specified frequency
// Writes samples directly to WAV file
// Returns false on error
bool writePulse(WavWriter *writer, uint16_t frequency, const WaveformConfig *config);

// =============================================================================
// MSX Bit/Byte Encoding - High Level
// =============================================================================

// Write a 0-bit (1 pulse at 1200 Hz)
bool writeBit0(WavWriter *writer, const WaveformConfig *config);

// Write a 1-bit (2 pulses at 2400 Hz)
bool writeBit1(WavWriter *writer, const WaveformConfig *config);

// Write a complete byte with serial framing
// (START bit + 8 data bits LSB first + 2 STOP bits = 11 bits total)
bool writeByte(WavWriter *writer, uint8_t byte, const WaveformConfig *config);

// =============================================================================
// MSX Tape Structure - Cassette Protocol
// =============================================================================

// Write silence (zero amplitude) for specified duration in seconds
bool writeSilence(WavWriter *writer, float seconds);

// Write sync header (consecutive 1-bits for synchronization)
bool writeSync(WavWriter *writer, size_t bit_count, const WaveformConfig *config);

// =============================================================================
// CAS to WAV Conversion - Complete File Processing
// =============================================================================

// Convert a complete CAS file to WAV audio format
// Reads the CAS file, parses its structure, and generates MSX cassette tape audio
// Returns true on success, false on error
bool convertCasToWav(const char *cas_filename, const char *wav_filename, 
                     const WaveformConfig *config, bool verbose);

#endif // WAVLIB_H
