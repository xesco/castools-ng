#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include "lib/presetlib.h"
#include "commands/commands.h"

// Forward declarations for command handlers
static int cmd_list(int argc, char *argv[]);
static int cmd_info(int argc, char *argv[]);
static int cmd_export(int argc, char *argv[]);

static int cmd_convert(int argc, char *argv[]);
static int cmd_profile(int argc, char *argv[]);
static int cmd_play(int argc, char *argv[]);

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
    {"convert", cmd_convert, "Convert CAS to WAV audio"},
    {"profile", cmd_profile, "List or show audio profiles"},
    {"play", cmd_play, "Play WAV file with marker display"},
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
    printf("  -D, --disk-format   Add MSX-DOS disk format markers for Binary files (0xFE/0xFF prefix and postfix)\n");
    printf("  -f, --force         Overwrite existing files\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -h, --help          Show this help message\n");
}

static void print_convert_help(void) {
    printf("Usage: cast convert <input.cas> <output.wav> [options]\n\n");
    printf("Convert CAS file to MSX cassette tape WAV audio.\n\n");
    printf("Options:\n");
    printf("  -b, --baud <rate>       Baud rate: 1200 (standard) or 2400 (turbo) [default: 1200]\n");
    printf("  -s, --sample <rate>     Sample rate in Hz [default: 43200]\n");
    printf("                          Common: 43200, 44100, 48000, 88200, 96000\n");
    printf("                          Must be divisible by 1200\n");
    printf("  -w, --wave <type>       Waveform type [default: sine]\n");
    printf("                          Types: sine, square, triangle, trapezoid\n");
    printf("  -r, --rise <percent>    Trapezoid rise/fall time (requires --wave trapezoid)\n");
    printf("                          Percentage of cycle: 1-50 [default: 10]\n");
    printf("                          Lower = sharper edges, Higher = gentler slopes\n");
    printf("  -t, --leader <preset>   Leader/silence timing preset [default: standard]\n");
    printf("                          standard: 2.0s/1.0s (default, fast loading)\n");
    printf("                          conservative: 3.0s/2.0s (more AGC/motor time)\n");
    printf("                          extended: 5.0s/3.0s (maximum compatibility)\n");
    printf("  -p, --profile <name>    Use predefined audio profile\n");
    printf("                          Use 'cast profile' to list available profiles\n");
    printf("                          Individual options override profile values\n");
    printf("  -c, --channels <num>    Channels: 1 (mono) or 2 (stereo) [default: 1]\n");
    printf("  -d, --depth <bits>      Bit depth: 8 or 16 [default: 8]\n");
    printf("  -a, --amplitude <val>   Signal amplitude: 1-127 for 8-bit, 1-255 for 16-bit [default: 120]\n");
    printf("  -l, --lowpass [freq]    Enable low-pass filter [default cutoff: 6000 Hz]\n");
    printf("                          Reduces harmonics for cleaner playback from computer\n");
    printf("                          Useful frequencies: 5000-7000 Hz (above max 4800 Hz signal)\n");
    printf("  -m, --markers           Add cue point markers to WAV file for timeline tracking\n");
    printf("                          Markers show file boundaries, silence, and sync signals\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -h, --help              Show this help message\n\n");
    printf("Examples:\n");
    printf("  cast convert game.cas game.wav\n");
    printf("  cast convert game.cas game.wav --baud 2400 --wave square\n");
    printf("  cast convert game.cas game.wav -s 44100 -a 100\n");
    printf("  cast convert game.cas game.wav --lowpass\n");
    printf("  cast convert game.cas game.wav --wave trapezoid --rise 20\n");
    printf("  cast convert game.cas game.wav --leader conservative\n");
    printf("  cast convert game.cas game.wav --profile computer-direct\n");
    printf("  cast convert game.cas game.wav --profile default --baud 2400\n");
    printf("  cast convert game.cas game.wav --lowpass 5500 --wave trapezoid\n");
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

    return execute_info(input_file, verbose);
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

static int cmd_convert(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    const char *profile_name = NULL;
    uint16_t baud_rate = 1200;
    uint32_t sample_rate = 43200;
    WaveformType waveform_type = WAVE_SQUARE;
    uint16_t channels = 1;
    uint16_t bits_per_sample = 8;
    uint8_t amplitude = 120;
    uint8_t trapezoid_rise_percent = 10;
    float long_silence = 2.0f;   // Standard timing
    float short_silence = 1.0f;
    bool enable_lowpass = false;
    uint16_t lowpass_cutoff_hz = 6000;
    bool enable_markers = false;
    bool verbose = false;
    
    // Track which options were explicitly set (for profile override)
    bool explicit_baud = false;
    bool explicit_sample = false;
    bool explicit_wave = false;
    bool explicit_rise = false;
    bool explicit_amplitude = false;
    bool explicit_leader = false;
    bool explicit_lowpass = false;

    struct option long_options[] = {
        {"baud", required_argument, 0, 'b'},
        {"sample", required_argument, 0, 's'},
        {"wave", required_argument, 0, 'w'},
        {"channels", required_argument, 0, 'c'},
        {"depth", required_argument, 0, 'd'},
        {"amplitude", required_argument, 0, 'a'},
        {"rise", required_argument, 0, 'r'},
        {"leader", required_argument, 0, 't'},
        {"profile", required_argument, 0, 'p'},
        {"lowpass", optional_argument, 0, 'l'},
        {"markers", no_argument, 0, 'm'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "b:s:w:c:d:a:r:t:p:l::mvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'b':
                baud_rate = atoi(optarg);
                explicit_baud = true;
                break;
            case 's':
                sample_rate = atoi(optarg);
                explicit_sample = true;
                break;
            case 'w':
                if (strcasecmp(optarg, "sine") == 0) {
                    waveform_type = WAVE_SINE;
                } else if (strcasecmp(optarg, "square") == 0) {
                    waveform_type = WAVE_SQUARE;
                } else if (strcasecmp(optarg, "triangle") == 0) {
                    waveform_type = WAVE_TRIANGLE;
                } else if (strcasecmp(optarg, "trapezoid") == 0) {
                    waveform_type = WAVE_TRAPEZOID;
                } else {
                    fprintf(stderr, "Error: Unknown waveform type '%s'\n", optarg);
                    fprintf(stderr, "Valid types: sine, square, triangle, trapezoid\n");
                    return 1;
                }
                explicit_wave = true;
                break;
            case 'c':
                channels = atoi(optarg);
                break;
            case 'd':
                bits_per_sample = atoi(optarg);
                break;
            case 'a':
                amplitude = atoi(optarg);
                explicit_amplitude = true;
                break;
            case 'r':
                trapezoid_rise_percent = atoi(optarg);
                if (trapezoid_rise_percent < 1 || trapezoid_rise_percent > 50) {
                    fprintf(stderr, "Error: Rise time must be between 1 and 50%%\n");
                    return 1;
                }
                explicit_rise = true;
                break;
            case 't':
                if (strcasecmp(optarg, "standard") == 0) {
                    long_silence = 2.0f;
                    short_silence = 1.0f;
                } else if (strcasecmp(optarg, "conservative") == 0) {
                    long_silence = 3.0f;
                    short_silence = 2.0f;
                } else if (strcasecmp(optarg, "extended") == 0) {
                    long_silence = 5.0f;
                    short_silence = 3.0f;
                } else {
                    fprintf(stderr, "Error: Unknown leader preset '%s'\n", optarg);
                    fprintf(stderr, "Valid presets: standard, conservative, extended\n");
                    return 1;
                }
                explicit_leader = true;
                break;
            case 'p':
                profile_name = optarg;
                break;
            case 'l':
                enable_lowpass = true;
                explicit_lowpass = true;
                if (optarg) {
                    lowpass_cutoff_hz = atoi(optarg);
                    if (lowpass_cutoff_hz == 0) {
                        fprintf(stderr, "Error: Invalid lowpass cutoff frequency\n");
                        return 1;
                    }
                }
                break;
            case 'm':
                enable_markers = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_convert_help();
                return 0;
            default:
                print_convert_help();
                return 1;
        }
    }

    // Get positional arguments
    if (optind + 2 > argc) {
        fprintf(stderr, "Error: Missing required arguments\n\n");
        print_convert_help();
        return 1;
    }

    input_file = argv[optind];
    output_file = argv[optind + 1];

    // Apply profile if specified
    if (profile_name) {
        const AudioProfile *profile = findProfile(profile_name);
        if (!profile) {
            fprintf(stderr, "Error: Unknown profile '%s'\n", profile_name);
            fprintf(stderr, "Use 'cast profile' to list available profiles.\n");
            return 1;
        }
        
        // Apply profile values only if not explicitly overridden
        if (!explicit_wave) waveform_type = profile->waveform;
        if (!explicit_baud) baud_rate = profile->baud_rate;
        if (!explicit_sample) sample_rate = profile->sample_rate;
        if (!explicit_amplitude) amplitude = profile->amplitude;
        if (!explicit_rise) trapezoid_rise_percent = profile->trapezoid_rise_percent;
        if (!explicit_leader) {
            long_silence = profile->long_silence;
            short_silence = profile->short_silence;
        }
        if (!explicit_lowpass) {
            enable_lowpass = profile->enable_lowpass;
            lowpass_cutoff_hz = profile->lowpass_cutoff_hz;
        }
        
        if (verbose) {
            printf("Using preset: %s\n", profile->name);
            printf("  %s\n\n", profile->short_desc);
        }
    }

    // Validate that --rise is only used with trapezoid waveform
    if (explicit_rise && waveform_type != WAVE_TRAPEZOID) {
        fprintf(stderr, "Error: --rise option requires --wave trapezoid\n");
        return 1;
    }

    return execute_convert(input_file, output_file, baud_rate, sample_rate,
                          waveform_type, channels, bits_per_sample, amplitude,
                          trapezoid_rise_percent,
                          long_silence, short_silence,
                          enable_lowpass, lowpass_cutoff_hz,
                          enable_markers, verbose);
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

static void print_profile_help(void) {
    printf("Usage: cast profile [<name>] [options]\n\n");
    printf("List or display audio profile presets.\n\n");
    printf("Without arguments:\n");
    printf("  Lists all available profiles with short descriptions\n\n");
    printf("With profile name:\n");
    printf("  Shows detailed information about the specified profile\n\n");
    printf("Options:\n");
    printf("  -v, --verbose           Show command examples\n");
    printf("  -h, --help              Show this help message\n\n");
    printf("Examples:\n");
    printf("  cast profile                    # List all profiles\n");
    printf("  cast profile computer-direct    # Show details for computer-direct\n");
    printf("  cast profile msx1 -v            # Show details with examples\n");
}

static int cmd_profile(int argc, char *argv[]) {
    const char *profile_name = NULL;
    bool verbose = false;
    
    struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    optind = 1;  // Reset getopt
    while ((opt = getopt_long(argc, argv, "vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_profile_help();
                return 0;
            default:
                print_profile_help();
                return 1;
        }
    }
    
    // Get profile name if provided
    if (optind < argc) {
        profile_name = argv[optind];
    }
    
    return execute_profile(profile_name, verbose);
}

static void print_play_help(void) {
    printf("Usage: cast play <file.wav> [options]\n\n");
    printf("Play a WAV file with real-time marker display.\n");
    printf("Shows loading progress, current file/block, and recent activity.\n\n");
    printf("Options:\n");
    printf("  -v, --verbose     Verbose output\n");
    printf("  -h, --help        Show this help message\n\n");
    printf("Interactive Controls:\n");
    printf("  Space       - Play/Pause\n");
    printf("  Left/Right  - Seek -5s/+5s\n");
    printf("  Up/Down     - Volume +10%%/-10%%\n");
    printf("  h           - Toggle help display\n");
    printf("  q           - Quit\n\n");
    printf("Examples:\n");
    printf("  cast play output.wav              # Play WAV file\n");
    printf("  cast play disc.wav -v             # Play with verbose output\n");
}

static int cmd_play(int argc, char *argv[]) {
    bool verbose = false;
    
    struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    optind = 1;  // Reset getopt
    while ((opt = getopt_long(argc, argv, "vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_play_help();
                return 0;
            default:
                print_play_help();
                return 1;
        }
    }
    
    // Check for input file
    if (optind >= argc) {
        fprintf(stderr, "Error: WAV file required\n\n");
        print_play_help();
        return 1;
    }
    
    const char *filename = argv[optind];
    return execute_play(filename, verbose);
}

