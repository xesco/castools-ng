#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <stdint.h>
#include "../lib/wavlib.h"

int execute_list(const char *input_file, bool extended, int filter_index, bool verbose);
int execute_export(const char *input_file, int filter_index, const char *output_dir, bool force, bool verbose, bool disk_format);
int execute_doctor(const char *input_file, bool check_disk_markers, bool verbose);
int execute_convert(const char *input_file, const char *output_file,
                    uint16_t baud_rate, uint32_t sample_rate, 
                    WaveformType waveform_type, uint16_t channels,
                    uint16_t bits_per_sample, uint8_t amplitude,
                    bool verbose);

#endif
