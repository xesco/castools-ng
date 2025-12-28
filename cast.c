#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include "commands/commands.h"

// Forward declarations for command handlers
static int cmd_list(int argc, char *argv[]);
static int cmd_info(int argc, char *argv[]);
static int cmd_export(int argc, char *argv[]);

// Command structure
typedef struct {
    const char *name;
    int (*handler)(int argc, char *argv[]);
    const char *description;
} Command;

// Available commands
static const Command commands[] = {
    {"list", cmd_list, "List files in a CAS container"},
    {"info", cmd_info, "Show container statistics"},
    {"export", cmd_export, "Export file(s) from container"},
    {NULL, NULL, NULL}
};

static void print_usage(const char *prog_name) {
    printf("Usage: %s <command> [options] <arguments>\n\n", prog_name);
    printf("Commands:\n");
    for (const Command *cmd = commands; cmd->name != NULL; cmd++) {
        printf("  %-12s %s\n", cmd->name, cmd->description);
    }
    printf("\nUse '%s <command> --help' for more information on a command.\n", prog_name);
}

static void print_list_help(void) {
    printf("Usage: cast list <file.cas> [options]\n\n");
    printf("Options:\n");
    printf("  -e, --extended      Show extended information (sizes, headers, data, etc..)\n");
    printf("  -i, --index <num>   Show only specific file by index (1-based, requires -e/--extended)\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -h, --help          Show this help message\n");
}

static void print_info_help(void) {
    printf("Usage: cast info <file.cas> [options]\n\n");
    printf("Options:\n");
    printf("  -v, --verbose     Verbose output\n");
    printf("  -h, --help        Show this help message\n");
}

static void print_export_help(void) {
    printf("Usage: cast export <file.cas> [options]\n\n");
    printf("Export files from a CAS container.\n");
    printf("By default, exports all files with auto-generated names.\n\n");
    printf("Options:\n");
    printf("  -i, --index <num>   Export only specific file by index (1-based)\n");
    printf("  -d, --dir <dir>     Output directory (default: current directory)\n");
    printf("  -D, --disk-format   Add MSX-DOS disk format headers (0xFE/0xFF prefix)\n");
    printf("  -f, --force         Overwrite existing files\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -h, --help          Show this help message\n");
}

static int cmd_list(int argc, char *argv[]) {
    const char *input_file = NULL;
    int filter_index = 0;
    bool extended = false;
    bool verbose = false;
    
    struct option long_options[] = {
        {"extended", no_argument, 0, 'e'},
        {"index", required_argument, 0, 'i'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    optind = 1; // Reset getopt
    while ((opt = getopt_long(argc, argv, "ei:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'e':
                extended = true;
                break;
            case 'i':
                filter_index = atoi(optarg);
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_list_help();
                return 0;
            default:
                return 1;
        }
    }
    
    if (optind >= argc) {
        print_list_help();
        return 0;
    }
    
    input_file = argv[optind];
    
    // Validate that -i requires -e
    if (filter_index && !extended) {
        fprintf(stderr, "Error: -i/--index option requires -e/--extended\n");
        return 1;
    }
    
    // Execute the list command
    return execute_list(input_file, extended, filter_index, verbose);
}

static int cmd_info(int argc, char *argv[]) {
    const char *input_file = NULL;
    bool verbose = false;
    
    struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    optind = 1;
    while ((opt = getopt_long(argc, argv, "vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_info_help();
                return 0;
            default:
                return 1;
        }
    }
    
    if (optind >= argc) {
        print_info_help();
        return 0;
    }
    
    input_file = argv[optind];
    
    // Print parsed options (boilerplate)
    printf("Command: info\n");
    printf("  Input file: %s\n", input_file);
    printf("  Verbose: %s\n", verbose ? "yes" : "no");
    
    // TODO: Implement actual info functionality
    printf("\n[TODO: Show statistics for %s]\n", input_file);
    
    return 0;
}

static int cmd_export(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_dir = NULL;
    int index = -1;
    bool force = false;
    bool verbose = false;
    bool disk_format = false;
    
    struct option long_options[] = {
        {"index", required_argument, 0, 'i'},
        {"dir", required_argument, 0, 'd'},
        {"disk-format", no_argument, 0, 'D'},
        {"force", no_argument, 0, 'f'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    optind = 1;
    while ((opt = getopt_long(argc, argv, "i:d:Dfvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i':
                index = atoi(optarg);
                break;
            case 'd':
                output_dir = optarg;
                break;
            case 'D':
                disk_format = true;
                break;
            case 'f':
                force = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_export_help();
                return 0;
            default:
                return 1;
        }
    }
    
    if (optind >= argc) {
        print_export_help();
        return 0;
    }
    
    input_file = argv[optind];
    
    // Call the execute_export implementation
    return execute_export(input_file, index, output_dir, force, verbose, disk_format);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Check for global --help or --version
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-V") == 0) {
        printf("cast version 1.0.0\n");
        return 0;
    }
    
    // Find and execute command
    const char *cmd_name = argv[1];
    for (const Command *cmd = commands; cmd->name != NULL; cmd++) {
        if (strcmp(cmd_name, cmd->name) == 0) {
            return cmd->handler(argc - 1, argv + 1);
        }
    }
    
    fprintf(stderr, "Error: Unknown command '%s'\n", cmd_name);
    fprintf(stderr, "Run '%s --help' for usage.\n", argv[0]);
    return 1;
}
