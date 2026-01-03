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
// WAV Cue Point Markers
// =============================================================================

// Marker categories for filtering during playback
typedef enum {
    MARKER_STRUCTURE,  // Essential structure: file/block boundaries
    MARKER_DETAIL,     // Includes structure + operational details
    MARKER_VERBOSE     // Everything including fine-grained progress
} MarkerCategory;

// Single cue point marker
typedef struct {
    size_t sample_position;     // Sample offset in WAV data
    MarkerCategory category;    // Category for filtering
    char description[256];      // Human-readable label
} Marker;

// Dynamic list of markers
typedef struct {
    Marker *markers;            // Array of markers
    size_t count;               // Number of markers in use
    size_t capacity;            // Allocated capacity
} MarkerList;

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
    uint32_t sample_rate;           // Sample rate in Hz (must be divisible by 1200)
    const uint8_t *custom_samples;  // For WAVE_CUSTOM: pre-calculated samples
    size_t custom_length;           // Number of samples in custom waveform
    
    // Trapezoid wave settings (only used when type == WAVE_TRAPEZOID)
    uint8_t trapezoid_rise_percent; // Rise/fall time as percentage (5, 10, 15, 20)
    
    // Silence/Leader timing settings (in seconds)
    float long_silence;             // Silence before file header (default: 2.0s)
    float short_silence;            // Silence before data blocks (default: 1.0s)
    
    // Low-pass filter settings
    bool enable_lowpass;            // Enable low-pass filtering
    uint16_t lowpass_cutoff_hz;     // Cutoff frequency in Hz (e.g., 6000)
    
    // Marker generation settings
    bool enable_markers;            // Generate cue point markers during conversion
} WaveformConfig;

// WAV file writer context (opaque to user)
typedef struct {
    FILE *file;
    WavFormat format;
    size_t sample_count;         // Total samples written (for position tracking)
    long data_chunk_pos;
    double lowpass_state;        // Filter state (previous output sample)
    MarkerList *markers;         // NULL if markers disabled
} WavWriter;

// =============================================================================
// Factory Functions - Easy Defaults
// =============================================================================

// Create default MSX WAV format (43200 Hz, 8-bit, mono, amplitude 120)
WavFormat createDefaultWavFormat(void);

// Create default waveform config (sine wave with default amplitude)
WaveformConfig createDefaultWaveform(void);

// =============================================================================
// Validation
// =============================================================================

// Validate WAV format settings
// Returns true if valid, false otherwise
bool validateWavFormat(const WavFormat *format);

// =============================================================================
// Marker Management
// =============================================================================

// Create a new empty marker list
// Returns NULL on allocation failure
MarkerList* createMarkerList(void);

// Add a marker to the list
// Returns false on allocation failure
// Description will be truncated to 255 characters if too long
bool addMarker(MarkerList *list, size_t sample_pos, 
               MarkerCategory category, const char *description);

// Free marker list and all associated memory
void freeMarkerList(MarkerList *list);

// Enable marker generation for a WAV writer
// Must be called after createWavFile() and before writing audio
// Returns false on allocation failure
bool enableMarkers(WavWriter *writer);

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
// Audio Processing - Filters
// =============================================================================

// Apply a simple single-pole IIR low-pass filter to audio samples
// This reduces high-frequency harmonics and smooths the waveform
//
// Parameters:
//   samples: Array of 8-bit unsigned audio samples to filter (modified in-place)
//   count: Number of samples in the array
//   sample_rate: Sample rate in Hz (e.g., 43200)
//   cutoff_hz: Cutoff frequency in Hz (e.g., 6000)
//              Frequencies above this will be attenuated
//   prev_output: Pointer to previous output sample (for filter state)
//                Initialize to 128 (8-bit center) before first call
//
// The filter uses a simple first-order IIR (Infinite Impulse Response) formula:
//   alpha = dt / (RC + dt)
//   output[n] = alpha * input[n] + (1 - alpha) * output[n-1]
//
// Where RC = 1 / (2π * cutoff_hz)
//
// This creates a smooth roll-off starting at the cutoff frequency,
// attenuating high frequencies while preserving the fundamental signal.
//
void applyLowPassFilter(uint8_t *samples, size_t count, 
                        uint32_t sample_rate, uint16_t cutoff_hz,
                        double *prev_output);

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
// If duration_seconds is not NULL, stores the WAV duration in seconds
bool convertCasToWav(const char *cas_filename, const char *wav_filename, 
                     const WaveformConfig *config, bool verbose, double *duration_seconds);

// =============================================================================
// Audio Estimation - Duration and Size Calculations
// =============================================================================

// Calculate estimated audio duration for a CAS container at specified baud rate
// This mimics the WAV generation logic including silence periods
// Returns duration in seconds
double calculateAudioDuration(const void *container, uint16_t baud_rate, 
                             float long_silence, float short_silence);

// Calculate estimated WAV file size for a given duration and sample rate
// Assumes 8-bit mono format
// Returns size in bytes (including 44-byte WAV header)
size_t calculateWavFileSize(double duration_seconds, uint32_t sample_rate);

#endif // WAVLIB_H
