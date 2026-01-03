#include "wavlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// =============================================================================
// WAV File Format Structures (RIFF/WAVE)
// =============================================================================

// WAV file header structures (all values are little-endian)
typedef struct {
    char riff[4];           // "RIFF"
    uint32_t file_size;     // File size - 8 bytes
    char wave[4];           // "WAVE"
} WavRiffHeader;

typedef struct {
    char fmt[4];            // "fmt "
    uint32_t chunk_size;    // 16 for PCM
    uint16_t audio_format;  // 1 for PCM
    uint16_t num_channels;  // 1 = mono, 2 = stereo
    uint32_t sample_rate;   // Samples per second
    uint32_t byte_rate;     // sample_rate * num_channels * bits_per_sample / 8
    uint16_t block_align;   // num_channels * bits_per_sample / 8
    uint16_t bits_per_sample; // 8 or 16
} WavFmtChunk;

typedef struct {
    char data[4];           // "data"
    uint32_t data_size;     // Size of audio data
} WavDataChunk;

// =============================================================================
// Helper Functions - Little Endian Writing
// =============================================================================

static void write_u32_le(uint8_t *buf, uint32_t value) {
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
}

// =============================================================================
// Factory Functions - Sensible Defaults
// =============================================================================

WavFormat createDefaultWavFormat(void) {
    WavFormat fmt = {
        .sample_rate = 43200,      // Divides evenly into 1200 and 2400 Hz
        .bits_per_sample = 8,      // 8-bit unsigned PCM
        .channels = 1,             // Mono
        .amplitude = 120           // Safe amplitude (leaves headroom)
    };
    return fmt;
}

WaveformConfig createDefaultWaveform(void) {
    WaveformConfig config = {
        .type = WAVE_SINE,
        .amplitude = 120,
        .baud_rate = 1200,  // Standard MSX baud rate
        .custom_samples = NULL,
        .custom_length = 0
    };
    return config;
}

WaveformConfig createWaveform(WaveformType type, uint8_t amplitude) {
    WaveformConfig config = {
        .type = type,
        .amplitude = amplitude,
        .baud_rate = 1200,  // Standard MSX baud rate
        .custom_samples = NULL,
        .custom_length = 0
    };
    return config;
}

// =============================================================================
// Validation
// =============================================================================

bool validateWavFormat(const WavFormat *format) {
    if (!format) return false;
    
    // Sample rate must divide evenly into MSX frequencies
    // Support both 1200 baud (1200/2400 Hz) and 2400 baud (2400/4800 Hz)
    if (format->sample_rate % 1200 != 0) {
        fprintf(stderr, "Error: Sample rate must be divisible by 1200 Hz\n");
        return false;
    }
    
    // Only 8-bit or 16-bit supported
    if (format->bits_per_sample != 8 && format->bits_per_sample != 16) {
        fprintf(stderr, "Error: Only 8-bit or 16-bit samples supported\n");
        return false;
    }
    
    // MSX uses mono only
    if (format->channels != 1) {
        fprintf(stderr, "Error: Only mono (1 channel) supported for MSX\n");
        return false;
    }
    
    // Amplitude must fit within bit depth
    if (format->bits_per_sample == 8 && format->amplitude > 127) {
        fprintf(stderr, "Error: Amplitude %d exceeds 8-bit limit (127)\n", format->amplitude);
        return false;
    }
    // Note: 16-bit check omitted - uint8_t amplitude can't exceed 255
    
    return true;
}

// =============================================================================
// WAV File Management
// =============================================================================

WavWriter* createWavFile(const char *filename, const WavFormat *format) {
    if (!filename || !format) {
        fprintf(stderr, "Error: Invalid parameters to createWavFile\n");
        return NULL;
    }
    
    // Validate format
    if (!validateWavFormat(format)) {
        return NULL;
    }
    
    // Allocate writer context
    WavWriter *writer = malloc(sizeof(WavWriter));
    if (!writer) {
        fprintf(stderr, "Error: Failed to allocate WavWriter\n");
        return NULL;
    }
    
    // Open file for writing
    writer->file = fopen(filename, "wb");
    if (!writer->file) {
        fprintf(stderr, "Error: Cannot create file '%s'\n", filename);
        free(writer);
        return NULL;
    }
    
    // Copy format and initialize counters
    writer->format = *format;
    writer->sample_count = 0;
    
    // Write WAV headers (with placeholder sizes - will update on close)
    WavRiffHeader riff = {
        .riff = {'R', 'I', 'F', 'F'},
        .file_size = 0,  // Will update on close
        .wave = {'W', 'A', 'V', 'E'}
    };
    
    WavFmtChunk fmt = {
        .fmt = {'f', 'm', 't', ' '},
        .chunk_size = 16,
        .audio_format = 1,  // PCM
        .num_channels = format->channels,
        .sample_rate = format->sample_rate,
        .byte_rate = format->sample_rate * format->channels * format->bits_per_sample / 8,
        .block_align = format->channels * format->bits_per_sample / 8,
        .bits_per_sample = format->bits_per_sample
    };
    
    WavDataChunk data = {
        .data = {'d', 'a', 't', 'a'},
        .data_size = 0  // Will update on close
    };
    
    // Write headers
    if (fwrite(&riff, sizeof(riff), 1, writer->file) != 1 ||
        fwrite(&fmt, sizeof(fmt), 1, writer->file) != 1 ||
        fwrite(&data, sizeof(data), 1, writer->file) != 1) {
        fprintf(stderr, "Error: Failed to write WAV headers\n");
        fclose(writer->file);
        free(writer);
        return NULL;
    }
    
    // Save position of data chunk size field for later update
    writer->data_chunk_pos = sizeof(WavRiffHeader) + sizeof(WavFmtChunk) + 4;
    
    return writer;
}

bool closeWavFile(WavWriter *writer) {
    if (!writer || !writer->file) {
        return false;
    }
    
    // Calculate sizes
    size_t bytes_per_sample = writer->format.bits_per_sample / 8;
    uint32_t data_size = writer->sample_count * bytes_per_sample;
    uint32_t file_size = 36 + data_size;  // 36 = header sizes before data
    
    // Update RIFF chunk size (at offset 4)
    fseek(writer->file, 4, SEEK_SET);
    uint8_t buf[4];
    write_u32_le(buf, file_size);
    fwrite(buf, 4, 1, writer->file);
    
    // Update data chunk size (at data_chunk_pos)
    fseek(writer->file, writer->data_chunk_pos, SEEK_SET);
    write_u32_le(buf, data_size);
    fwrite(buf, 4, 1, writer->file);
    
    // Close and free
    fclose(writer->file);
    free(writer);
    
    return true;
}

bool writeSamples(WavWriter *writer, const uint8_t *samples, size_t count) {
    if (!writer || !writer->file || !samples) {
        return false;
    }
    
    if (count == 0) {
        return true;
    }
    
    // Write samples
    if (fwrite(samples, 1, count, writer->file) != count) {
        fprintf(stderr, "Error: Failed to write samples to WAV file\n");
        return false;
    }
    
    writer->sample_count += count;
    return true;
}

// =============================================================================
// MSX Tape Structure
// =============================================================================

bool writeSilence(WavWriter *writer, float seconds) {
    if (!writer || seconds < 0) {
        return false;
    }
    
    size_t num_samples = (size_t)(writer->format.sample_rate * seconds);
    uint8_t silence_value = (writer->format.bits_per_sample == 8) ? 128 : 0;
    
    // Write silence in chunks to avoid huge allocations
    uint8_t buffer[4096];
    memset(buffer, silence_value, sizeof(buffer));
    
    while (num_samples > 0) {
        size_t chunk = (num_samples > sizeof(buffer)) ? sizeof(buffer) : num_samples;
        if (!writeSamples(writer, buffer, chunk)) {
            return false;
        }
        num_samples -= chunk;
    }
    
    return true;
}

// =============================================================================
// Waveform Generation - Pulse Primitives
// =============================================================================

// Generate one complete wave cycle (pulse) at specified frequency
bool writePulse(WavWriter *writer, uint16_t frequency, const WaveformConfig *config) {
    if (!writer || !config || frequency == 0) {
        return false;
    }
    
    // Calculate samples per complete cycle
    size_t samples_per_cycle = writer->format.sample_rate / frequency;
    
    if (samples_per_cycle == 0) {
        fprintf(stderr, "Error: Frequency %u Hz too high for sample rate %u Hz\n", 
                frequency, writer->format.sample_rate);
        return false;
    }
    
    // Allocate buffer for one complete cycle
    uint8_t *buffer = malloc(samples_per_cycle);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate pulse buffer\n");
        return false;
    }
    
    // Generate waveform based on type
    switch (config->type) {
        case WAVE_SINE:
            // Sine wave: amplitude * sin(2π * t)
            for (size_t i = 0; i < samples_per_cycle; i++) {
                double t = (double)i / samples_per_cycle;  // 0.0 to 1.0
                double phase = 2.0 * M_PI * t;
                double sample = sin(phase);
                // Scale to amplitude and offset for unsigned 8-bit (128 = center)
                buffer[i] = (uint8_t)(128 + config->amplitude * sample);
            }
            break;
            
        case WAVE_SQUARE:
            // Square wave: first half high, second half low
            for (size_t i = 0; i < samples_per_cycle; i++) {
                if (i < samples_per_cycle / 2) {
                    buffer[i] = 128 + config->amplitude;  // High
                } else {
                    buffer[i] = 128 - config->amplitude;  // Low
                }
            }
            break;
            
        case WAVE_TRIANGLE:
            // Triangle wave: linear ramp up, then linear ramp down
            for (size_t i = 0; i < samples_per_cycle; i++) {
                double t = (double)i / samples_per_cycle;
                double sample;
                if (t < 0.5) {
                    // Ramp up: 0.0 to 1.0
                    sample = 4.0 * t - 1.0;
                } else {
                    // Ramp down: 1.0 to 0.0
                    sample = 3.0 - 4.0 * t;
                }
                buffer[i] = (uint8_t)(128 + config->amplitude * sample);
            }
            break;
            
        case WAVE_TRAPEZOID: {
            // Trapezoid: like square but with 10% rise/fall time on each edge
            size_t rise_samples = samples_per_cycle / 10;
            if (rise_samples < 1) rise_samples = 1;
            
            for (size_t i = 0; i < samples_per_cycle; i++) {
                double sample;
                size_t half = samples_per_cycle / 2;
                
                if (i < rise_samples) {
                    // Rising edge (0 to 1)
                    sample = (double)i / rise_samples;
                } else if (i < half - rise_samples) {
                    // High plateau
                    sample = 1.0;
                } else if (i < half + rise_samples) {
                    // Falling edge (1 to -1)
                    double t = (double)(i - (half - rise_samples)) / (2.0 * rise_samples);
                    sample = 1.0 - 2.0 * t;
                } else if (i < samples_per_cycle - rise_samples) {
                    // Low plateau
                    sample = -1.0;
                } else {
                    // Rising edge (-1 to 0)
                    double t = (double)(i - (samples_per_cycle - rise_samples)) / rise_samples;
                    sample = -1.0 + t;
                }
                
                buffer[i] = (uint8_t)(128 + config->amplitude * sample);
            }
            break;
        }
            
        case WAVE_CUSTOM:
            // Custom waveform: repeat user-provided samples to fill one cycle
            if (!config->custom_samples || config->custom_length == 0) {
                fprintf(stderr, "Error: Custom waveform requires samples\n");
                free(buffer);
                return false;
            }
            for (size_t i = 0; i < samples_per_cycle; i++) {
                // Map cycle position to custom sample array (with wrapping)
                size_t custom_idx = (i * config->custom_length) / samples_per_cycle;
                buffer[i] = config->custom_samples[custom_idx];
            }
            break;
            
        default:
            fprintf(stderr, "Error: Unknown waveform type\n");
            free(buffer);
            return false;
    }
    
    // Write the generated waveform
    bool result = writeSamples(writer, buffer, samples_per_cycle);
    free(buffer);
    
    return result;
}

// =============================================================================
// MSX Bit Encoding - FSK (Frequency Shift Keying)
// =============================================================================

// Write a 0-bit: 1 pulse at baud_rate Hz (1200 Hz for standard, 2400 Hz for turbo)
bool writeBit0(WavWriter *writer, const WaveformConfig *config) {
    if (!writer || !config) {
        return false;
    }
    
    // MSX 0-bit = one complete pulse at baud_rate Hz
    return writePulse(writer, config->baud_rate, config);
}

// Write a 1-bit: 2 pulses at 2×baud_rate Hz (2400 Hz for standard, 4800 Hz for turbo)
bool writeBit1(WavWriter *writer, const WaveformConfig *config) {
    if (!writer || !config) {
        return false;
    }
    
    // MSX 1-bit = two complete pulses at 2×baud_rate Hz
    uint16_t freq = config->baud_rate * 2;
    if (!writePulse(writer, freq, config)) {
        return false;
    }
    return writePulse(writer, freq, config);
}

// =============================================================================
// MSX Byte Encoding - Serial Framing
// =============================================================================

// Write a byte with MSX serial framing: START + 8 data bits (LSB-first) + 2 STOP
bool writeByte(WavWriter *writer, uint8_t byte, const WaveformConfig *config) {
    if (!writer || !config) {
        return false;
    }
    
    // START bit (always 0)
    if (!writeBit0(writer, config)) {
        return false;
    }
    
    // 8 data bits, LSB first
    for (int i = 0; i < 8; i++) {
        int bit = (byte >> i) & 1;
        if (bit) {
            if (!writeBit1(writer, config)) {
                return false;
            }
        } else {
            if (!writeBit0(writer, config)) {
                return false;
            }
        }
    }
    
    // 2 STOP bits (always 1)
    if (!writeBit1(writer, config)) {
        return false;
    }
    if (!writeBit1(writer, config)) {
        return false;
    }
    
    return true;
}

// =============================================================================
// MSX Tape Protocol - Sync/Header Sequences
// =============================================================================

// Write sync pulses (consecutive 1-bits)
// MSX uses: 8000 pulses for file headers, 2000 for data blocks
bool writeSync(WavWriter *writer, size_t count, const WaveformConfig *config) {
    if (!writer || !config) {
        return false;
    }
    
    // Write the specified number of consecutive 1-bits
    for (size_t i = 0; i < count; i++) {
        if (!writeBit1(writer, config)) {
            return false;
        }
    }
    
    return true;
}

// =============================================================================
// CAS to WAV Conversion - Complete File Processing
// =============================================================================

#include "caslib.h"
#include <sys/stat.h>

// Helper: Write file header block (type marker + filename)
static bool writeFileHeaderBlock(WavWriter *writer, const cas_File *file, 
                                 const WaveformConfig *config) {
    // Write type marker (10 bytes)
    for (int i = 0; i < 10; i++) {
        if (!writeByte(writer, file->file_header.file_type[i], config)) {
            return false;
        }
    }
    
    // Write filename (6 bytes)
    for (int i = 0; i < 6; i++) {
        if (!writeByte(writer, file->file_header.file_name[i], config)) {
            return false;
        }
    }
    
    return true;
}

// Helper: Write data block header (for BINARY/BASIC files)
static bool writeDataBlockHeader(WavWriter *writer, const cas_File *file,
                                 const WaveformConfig *config) {
    // Write load address (2 bytes, little-endian)
    uint16_t load_addr = file->data_block_header.load_address;
    if (!writeByte(writer, load_addr & 0xFF, config)) return false;
    if (!writeByte(writer, (load_addr >> 8) & 0xFF, config)) return false;
    
    // Write end address (2 bytes, little-endian)
    uint16_t end_addr = file->data_block_header.end_address;
    if (!writeByte(writer, end_addr & 0xFF, config)) return false;
    if (!writeByte(writer, (end_addr >> 8) & 0xFF, config)) return false;
    
    // Write exec address (2 bytes, little-endian)
    uint16_t exec_addr = file->data_block_header.exec_address;
    if (!writeByte(writer, exec_addr & 0xFF, config)) return false;
    if (!writeByte(writer, (exec_addr >> 8) & 0xFF, config)) return false;
    
    return true;
}

// Convert a complete CAS file to WAV audio format
bool convertCasToWav(const char *cas_filename, const char *wav_filename,
                     const WaveformConfig *config, bool verbose) {
    if (!cas_filename || !wav_filename || !config) {
        fprintf(stderr, "Error: Invalid parameters to convertCasToWav\n");
        return false;
    }
    
    // Read CAS file into memory
    FILE *cas_file = fopen(cas_filename, "rb");
    if (!cas_file) {
        fprintf(stderr, "Error: Cannot open CAS file '%s'\n", cas_filename);
        return false;
    }
    
    // Get file size
    fseek(cas_file, 0, SEEK_END);
    size_t cas_size = ftell(cas_file);
    fseek(cas_file, 0, SEEK_SET);
    
    if (cas_size == 0) {
        fprintf(stderr, "Error: CAS file is empty\n");
        fclose(cas_file);
        return false;
    }
    
    // Allocate buffer and read file
    uint8_t *cas_data = malloc(cas_size);
    if (!cas_data) {
        fprintf(stderr, "Error: Failed to allocate memory for CAS file\n");
        fclose(cas_file);
        return false;
    }
    
    if (fread(cas_data, 1, cas_size, cas_file) != cas_size) {
        fprintf(stderr, "Error: Failed to read CAS file\n");
        free(cas_data);
        fclose(cas_file);
        return false;
    }
    fclose(cas_file);
    
    // Parse CAS container
    cas_Container container;
    if (!parseCasContainer(cas_data, &container, cas_size)) {
        fprintf(stderr, "Error: Failed to parse CAS file\n");
        free(cas_data);
        return false;
    }
    
    if (verbose) {
        printf("Converting '%s' to '%s'...\n", cas_filename, wav_filename);
        printf("  Files in container: %zu\n", container.file_count);
    }
    
    // Create WAV file
    WavFormat format = createDefaultWavFormat();
    WavWriter *writer = createWavFile(wav_filename, &format);
    if (!writer) {
        fprintf(stderr, "Error: Failed to create WAV file\n");
        free(cas_data);
        return false;
    }
    
    // Process each file in the container
    for (size_t file_idx = 0; file_idx < container.file_count; file_idx++) {
        const cas_File *file = &container.files[file_idx];
        
        if (verbose) {
            printf("  File %zu/%zu: %s ", file_idx + 1, container.file_count,
                   getFileTypeString(file));
            if (!file->is_custom) {
                printf("\"%.6s\" ", (char*)file->file_header.file_name);
            }
            size_t total_blocks = file->is_custom ? file->data_block_count : (file->data_block_count + 1);
            printf("(%zu blocks)\n", total_blocks);
        }
        
        // BLOCK 1: File header block (only for non-custom files)
        if (!file->is_custom) {
            if (verbose) {
                printf("    Writing file header block...\n");
            }
            writeSilence(writer, 2.0);  // Long silence
            writeSync(writer, 8000, config);  // Initial sync (LONG_HEADER = 16000 pulses / 2)
            writeFileHeaderBlock(writer, file, config);
        }
        
        // Data blocks
        for (size_t block_idx = 0; block_idx < file->data_block_count; block_idx++) {
            const cas_DataBlock *block = &file->data_blocks[block_idx];
            
            if (verbose) {
                printf("    Writing data block %zu/%zu (%zu bytes)...\n",
                       block_idx + 1, file->data_block_count, block->data_size);
            }
            
            // For custom blocks or subsequent data blocks, use short header
            // For first data block of non-custom files, use short header too
            writeSilence(writer, 1.0);  // Short silence
            writeSync(writer, 2000, config);  // Block sync (SHORT_HEADER = 4000 pulses / 2)
            
            // For BINARY/BASIC files, write data block header first
            if (block_idx == 0 && (isBinaryFile(file->file_header.file_type) ||
                                   isBasicFile(file->file_header.file_type))) {
                writeDataBlockHeader(writer, file, config);
            }
            
            // Write all data bytes in this block
            for (size_t i = 0; i < block->data_size; i++) {
                if (!writeByte(writer, block->data[i], config)) {
                    fprintf(stderr, "Error: Failed to write data byte\n");
                    closeWavFile(writer);
                    free(cas_data);
                    return false;
                }
            }
        }
    }
    
    // Close WAV file
    if (!closeWavFile(writer)) {
        fprintf(stderr, "Error: Failed to close WAV file\n");
        free(cas_data);
        return false;
    }
    
    free(cas_data);
    return true;
}
