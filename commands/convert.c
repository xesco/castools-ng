#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../lib/caslib.h"
#include "../lib/wavlib.h"
#include "../lib/cmdlib.h"

// Validate sample rate (must be divisible by 1200)
static bool validateSampleRate(uint32_t rate) {
    if (rate % 1200 != 0) {
        fprintf(stderr, "Error: Sample rate must be divisible by 1200 Hz\n");
        fprintf(stderr, "       Common rates: 43200, 44100, 48000, 88200, 96000\n");
        return false;
    }
    if (rate < 1200 || rate > 192000) {
        fprintf(stderr, "Error: Sample rate must be between 1200 and 192000 Hz\n");
        return false;
    }
    return true;
}

// Validate baud rate
static bool validateBaudRate(uint16_t baud) {
    // Allow standard (1200), turbo (2400), and experimental higher rates
    if (baud < 1200 || baud > 9600) {
        fprintf(stderr, "Error: Baud rate must be between 1200-9600\n");
        return false;
    }
    return true;
}

// Validate amplitude
static bool validateAmplitude(uint8_t amplitude, uint16_t bits_per_sample) {
    uint8_t max_amplitude = (bits_per_sample == 8) ? 127 : 255;
    if (amplitude > max_amplitude) {
        fprintf(stderr, "Error: Amplitude %u exceeds %u-bit limit (%u)\n",
                amplitude, bits_per_sample, max_amplitude);
        return false;
    }
    if (amplitude == 0) {
        fprintf(stderr, "Error: Amplitude must be greater than 0\n");
        return false;
    }
    return true;
}

// Validate bit depth
static bool validateBitDepth(uint16_t bits) {
    if (bits != 8 && bits != 16) {
        fprintf(stderr, "Error: Bit depth must be 8 or 16\n");
        return false;
    }
    return true;
}

// Validate channels
static bool validateChannels(uint16_t channels) {
    if (channels != 1 && channels != 2) {
        fprintf(stderr, "Error: Channels must be 1 (mono) or 2 (stereo)\n");
        return false;
    }
    if (channels == 2) {
        fprintf(stderr, "Warning: MSX uses mono audio. Stereo output will duplicate the signal.\n");
    }
    return true;
}

int execute_convert(const char *input_file, const char *output_file,
                    uint16_t baud_rate, uint32_t sample_rate, 
                    WaveformType waveform_type, uint16_t channels,
                    uint16_t bits_per_sample, uint8_t amplitude,
                    uint8_t trapezoid_rise_percent,
                    float long_silence, float short_silence,
                    bool enable_lowpass, uint16_t lowpass_cutoff_hz,
                    bool enable_markers, bool verbose) {
    
    // Validate all parameters
    if (!validateBaudRate(baud_rate)) return 1;
    if (!validateSampleRate(sample_rate)) return 1;
    if (!validateBitDepth(bits_per_sample)) return 1;
    if (!validateChannels(channels)) return 1;
    if (!validateAmplitude(amplitude, bits_per_sample)) return 1;
    
    if (verbose) {
        printf("=== CAS to WAV Conversion ===\n");
        printf("Input:  %s\n", input_file);
        printf("Output: %s\n\n", output_file);
        
        printf("Audio Settings:\n");
        printf("  Baud rate:     %u baud (%s)\n", baud_rate, 
               baud_rate == 1200 ? "standard" : "turbo");
        printf("  Sample rate:   %u Hz\n", sample_rate);
        printf("  Bit depth:     %u-bit\n", bits_per_sample);
        printf("  Channels:      %u (%s)\n", channels, 
               channels == 1 ? "mono" : "stereo");
        printf("  Amplitude:     %u\n", amplitude);
        printf("  Waveform:      ");
        switch (waveform_type) {
            case WAVE_SINE: printf("sine\n"); break;
            case WAVE_SQUARE: printf("square\n"); break;
            case WAVE_TRIANGLE: printf("triangle\n"); break;
            case WAVE_TRAPEZOID: 
                printf("trapezoid (rise: %u%%)\n", trapezoid_rise_percent); 
                break;
            default: printf("unknown\n"); break;
        }
        printf("  Low-pass:      %s", enable_lowpass ? "enabled" : "disabled");
        if (enable_lowpass) {
            printf(" (cutoff: %u Hz)", lowpass_cutoff_hz);
        }
        printf("\n");
        printf("  Leader timing: %.1fs / %.1fs (long/short)\n", long_silence, short_silence);
        printf("  Cue markers:   %s\n", enable_markers ? "enabled" : "disabled");
        printf("\n");
    }
    
    // Configure WAV format
    WavFormat format;
    format.sample_rate = sample_rate;
    format.bits_per_sample = bits_per_sample;
    format.channels = channels;
    format.amplitude = amplitude;
    
    // Validate the format
    if (!validateWavFormat(&format)) {
        return 1;
    }
    
    // Configure waveform
    WaveformConfig waveform;
    waveform.type = waveform_type;
    waveform.amplitude = amplitude;
    waveform.baud_rate = baud_rate;
    waveform.sample_rate = sample_rate;
    waveform.custom_samples = NULL;
    waveform.custom_length = 0;
    waveform.trapezoid_rise_percent = trapezoid_rise_percent;
    waveform.long_silence = long_silence;
    waveform.short_silence = short_silence;
    waveform.enable_lowpass = enable_lowpass;
    waveform.lowpass_cutoff_hz = lowpass_cutoff_hz;
    waveform.enable_markers = enable_markers;
    
    // Read and verify CAS file first
    size_t file_size;
    uint8_t *file_data = readFileIntoMemory(input_file, &file_size);
    if (!file_data) {
        fprintf(stderr, "Error: Failed to read file '%s'\n", input_file);
        return 1;
    }
    
    if (verbose) {
        printf("CAS file: %zu bytes\n", file_size);
    }
    
    // Parse CAS container to show what we're converting
    cas_Container container;
    if (!parseCasContainer(file_data, &container, file_size)) {
        fprintf(stderr, "Error: Failed to parse CAS file\n");
        free(file_data);
        return 1;
    }
    
    if (verbose) {
        printf("Files in container: %zu\n", container.file_count);
        for (size_t i = 0; i < container.file_count; i++) {
            const cas_File *file = &container.files[i];
            printf("  %zu. %s", i + 1, getFileTypeString(file));
            if (!file->is_custom) {
                printf(" \"%.6s\"", (char*)file->file_header.file_name);
            }
            
            size_t total_data = 0;
            for (size_t j = 0; j < file->data_block_count; j++) {
                total_data += file->data_blocks[j].data_size;
            }
            printf(" (%zu bytes)\n", total_data);
        }
        printf("\n");
    }
    
    // Store container for later command generation
    cas_Container saved_container = container;
    
    // Perform conversion
    double duration = 0.0;
    if (!convertCasToWav(input_file, output_file, &waveform, verbose, &duration)) {
        fprintf(stderr, "Error: Conversion failed\n");
        free(container.files);
        free(file_data);
        return 1;
    }
    
    // Success message with duration
    int minutes = (int)(duration / 60.0);
    int seconds = (int)(duration) % 60;
    printf("✓ Conversion complete!\n");
    printf("Audio length: %d:%02d (%.1f seconds)\n", minutes, seconds, duration);
    
    // Generate MSX command for loading (find first non-custom file)
    printf("MSX Command: ");
    
    bool found_command = false;
    for (size_t i = 0; i < saved_container.file_count; i++) {
        const cas_File *file = &saved_container.files[i];
        
        if (!file->is_custom && !found_command) {
            if (isAsciiFile(file->file_header.file_type) || isBasicFile(file->file_header.file_type)) {
                // Both ASCII and BASIC files use RUN"CAS:",R to load and auto-run
                printf("RUN\"CAS:\",R\n");
            } else if (isBinaryFile(file->file_header.file_type)) {
                if (file->data_block_header.exec_address != 0) {
                    printf("BLOAD\"CAS:\",R\n");
                } else {
                    printf("BLOAD\"CAS:\"\n");
                }
            }
            found_command = true;
            break;
        }
    }
    
    if (!found_command) {
        if (verbose) {
            printf("(Custom format - no standard MSX load command)\n");
        } else {
            printf("(Custom format)\n");
        }
    }
    
    free(saved_container.files);
    free(file_data);
    
    return 0;
}
