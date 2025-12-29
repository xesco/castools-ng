#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>

int execute_list(const char *input_file, bool extended, int filter_index, bool verbose);
int execute_export(const char *input_file, int filter_index, const char *output_dir, bool force, bool verbose, bool disk_format);
int execute_doctor(const char *input_file, bool check_disk_markers, bool verbose);

#endif
