#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <stdint.h>
#include "../lib/wavlib.h"

int execute_list(const char *input_file, bool extended, int filter_index, bool show_markers, bool verbose);
int execute_info(const char *input_file, bool verbose);
int execute_export(const char *input_file, int filter_index, const char *output_dir, bool force, bool verbose, bool disk_format);
int execute_convert(const char *input_file, const char *output_file,
                    uint16_t baud_rate, uint32_t sample_rate,
                    WaveformType waveform_type, uint16_t channels,
                    uint16_t bits_per_sample, uint8_t amplitude,
                    uint8_t trapezoid_rise_percent,
                    float long_silence, float short_silence,
                    bool enable_lowpass, uint16_t lowpass_cutoff_hz,
                    bool enable_markers, bool verbose);
int execute_profile(const char *profile_name, bool verbose);
int execute_play(const char *filename, bool verbose);

#endif // COMMANDS_H
