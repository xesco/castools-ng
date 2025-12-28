#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include "commands/commands.h"

// Forward declarations for command handlers
static int cmd_list(int argc, char *argv[]);
static int cmd_info(int argc, char *argv[]);
static int cmd_extract(int argc, char *argv[]);

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
    {"extract", cmd_extract, "Extract file(s) from container"},
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
                printf("Usage: cast list <file.cas> [options]\n\n");
                printf("Options:\n");
                printf("  -e, --extended      Show extended information (sizes, headers, data, etc..)\n");
                printf("  -i, --index <num>   Show only specific file by index (1-based, requires -e/--extended)\n");
                printf("  -v, --verbose       Verbose output\n");
                printf("  -h, --help          Show this help message\n");
                return 0;
            default:
                return 1;
        }
    }
    
    if (optind >= argc) {
        fprintf(stderr, "Error: Missing required argument <file.cas>\n");
        fprintf(stderr, "Usage: cast list <file.cas> [options]\n");
        return 1;
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
                printf("Usage: cast info <file.cas> [options]\n\n");
                printf("Options:\n");
                printf("  -v, --verbose     Verbose output\n");
                printf("  -h, --help        Show this help message\n");
                return 0;
            default:
                return 1;
        }
    }
    
    if (optind >= argc) {
        fprintf(stderr, "Error: Missing required argument <file.cas>\n");
        fprintf(stderr, "Usage: cast info <file.cas> [options]\n");
        return 1;
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

static int cmd_extract(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *file_name = NULL;
    const char *output_file = NULL;
    const char *output_dir = NULL;
    int index = -1;
    bool extract_all = false;
    bool force = false;
    bool verbose = false;
    
    struct option long_options[] = {
        {"name", required_argument, 0, 'n'},
        {"index", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"dir", required_argument, 0, 'd'},
        {"all", no_argument, 0, 'a'},
        {"force", no_argument, 0, 'f'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    optind = 1;
    while ((opt = getopt_long(argc, argv, "n:i:o:d:afvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'n':
                file_name = optarg;
                break;
            case 'i':
                index = atoi(optarg);
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'd':
                output_dir = optarg;
                break;
            case 'a':
                extract_all = true;
                break;
            case 'f':
                force = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                printf("Usage: cast extract <file.cas> [options]\n\n");
                printf("Options:\n");
                printf("  -n, --name <name>   Extract file by name\n");
                printf("  -i, --index <num>   Extract file by index (0-based)\n");
                printf("  -o, --output <file> Output filename\n");
                printf("  -d, --dir <dir>     Output directory (for --all)\n");
                printf("  -a, --all           Extract all files\n");
                printf("  -f, --force         Overwrite existing files\n");
                printf("  -v, --verbose       Verbose output\n");
                printf("  -h, --help          Show this help message\n");
                return 0;
            default:
                return 1;
        }
    }
    
    if (optind >= argc) {
        fprintf(stderr, "Error: Missing required argument <file.cas>\n");
        fprintf(stderr, "Usage: cast extract <file.cas> [options]\n");
        return 1;
    }
    
    input_file = argv[optind];
    
    // Validate options
    if (!extract_all && !file_name && index < 0) {
        fprintf(stderr, "Error: Must specify --name, --index, or --all\n");
        return 1;
    }
    
    // Print parsed options (boilerplate)
    printf("Command: extract\n");
    printf("  Input file: %s\n", input_file);
    if (file_name) printf("  File name: %s\n", file_name);
    if (index >= 0) printf("  File index: %d\n", index);
    if (output_file) printf("  Output file: %s\n", output_file);
    if (output_dir) printf("  Output directory: %s\n", output_dir);
    printf("  Extract all: %s\n", extract_all ? "yes" : "no");
    printf("  Force: %s\n", force ? "yes" : "no");
    printf("  Verbose: %s\n", verbose ? "yes" : "no");
    
    // TODO: Implement actual extract functionality
    printf("\n[TODO: Extract from %s]\n", input_file);
    
    return 0;
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
