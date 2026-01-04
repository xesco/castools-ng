/*
 * play.c - Play WAV file with real-time marker display
 */

#include "../lib/uilib.h"
#include "../lib/playlib.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

// =============================================================================
// Display State
// =============================================================================

typedef struct {
    char description[256];
    double time;
} RecentMarker;

typedef struct {
    const MarkerInfo *current_file;
    const MarkerInfo *current_block;
    const MarkerInfo *current_activity;  // Most recent marker of any type
    RecentMarker detail_markers[16];
    int detail_count;
} DisplayState;

// =============================================================================
// Display State Management
// =============================================================================

static void updateDisplayState(DisplayState *state, const MarkerListInfo *markers, double current_time) {
    if (!markers) return;

    state->current_file = NULL;
    state->current_block = NULL;
    state->current_activity = NULL;
    state->detail_count = 0;

    RecentMarker temp_details[100];
    int temp_count = 0;

    // Process all markers up to current time
    for (size_t i = 0; i < markers->count; i++) {
        const MarkerInfo *m = &markers->markers[i];

        if (m->time_seconds > current_time) break;

        // Track most recent activity (any marker)
        state->current_activity = m;

        // Track file and block markers
        if (m->category == MARKER_STRUCTURE) {
            const char *desc = m->description;
            // File marker: "File 1/3: ..."
            if (strstr(desc, "File ") && strstr(desc, "/") && strstr(desc, ":")) {
                state->current_file = m;
            }
            // Data block marker: "Data block 1/1 (256 bytes)"
            else if (strstr(desc, "Data block ")) {
                state->current_block = m;
            }
            // Skip other structure markers like "File header"
        }
        // Clear file when we hit silence after data blocks (preparation for next file)
        else if (m->category == MARKER_DETAIL && state->current_block) {
            const char *desc = m->description;
            // If we see silence after a data block, the current file is done
            if (strstr(desc, "Silence")) {
                state->current_file = NULL;
                state->current_block = NULL;
            }
        }

        // Collect ALL markers for event log (both STRUCTURE and DETAIL)
        if (temp_count < 100) {
            strncpy(temp_details[temp_count].description, m->description, 255);
            temp_details[temp_count].description[255] = '\0';
            temp_details[temp_count].time = m->time_seconds;
            temp_count++;
        }
    }

    // Keep only last MAX_ACTIVITIES entries
    int start_idx = (temp_count > MAX_ACTIVITIES) ? (temp_count - MAX_ACTIVITIES) : 0;
    state->detail_count = temp_count - start_idx;
    for (int i = 0; i < state->detail_count; i++) {
        state->detail_markers[i] = temp_details[start_idx + i];
    }
}

// =============================================================================
// Render Function
// =============================================================================

static void renderDisplay(AudioPlayer *player, DisplayState *state, const MarkerListInfo *markers, bool show_help) {
    tb_clear();
    int y = 0;

    // Capture current playback position ONCE at start of render to keep all displays in sync
    double current = getPlaybackPosition(player);
    double total = getAudioDuration(player);
    if (total <= 0) total = 1.0;  // Avoid division by zero

    // If help is shown, display help overlay and return
    if (show_help) {
        // Draw help box (centered, 60 chars wide)
        int box_left = 20;
        int box_right = 80;

        y = 3;
        draw_box_top(y++, box_left, box_right, COLOR_BORDER);
        
        draw_box_line(y++, box_left, box_right, "MSX Tape Player - Keyboard Shortcuts", COLOR_TITLE);
        
        draw_box_separator(y++, box_left, box_right, COLOR_BORDER);

        // Help content
        const char *help_lines[] = {
            "  SPACE      - Play / Pause",
            "  UP/DOWN    - Volume +/-",
            "  LEFT/RIGHT - Seek -/+ 5 seconds",
            "  H          - Toggle this help",
            "  Q or ESC   - Quit",
        };

        for (size_t i = 0; i < sizeof(help_lines) / sizeof(help_lines[0]); i++) {
            draw_box_line(y++, box_left, box_right, help_lines[i], COLOR_VALUE);
        }

        draw_box_line(y++, box_left, box_right, NULL, TB_WHITE);  // Empty line
        
        draw_box_line(y++, box_left, box_right, "Press 'h' again to close help...", COLOR_DIM);

        draw_box_bottom(y, box_left, box_right, COLOR_BORDER);

        tb_present();
        return;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Top border
    // ═══════════════════════════════════════════════════════════════════════
    tb_set_cell(0, y, 0x2554, TB_CYAN | TB_BOLD, TB_BLACK);  // ╔
    draw_hline(y, 1, SPLIT_COL, 0x2550, TB_CYAN | TB_BOLD);  // ═
    tb_set_cell(SPLIT_COL, y, 0x2566, TB_CYAN | TB_BOLD, TB_BLACK);  // ╦
    draw_hline(y, SPLIT_COL + 1, TOTAL_WIDTH - 1, 0x2550, TB_CYAN | TB_BOLD);  // ═
    tb_set_cell(TOTAL_WIDTH - 1, y, 0x2557, TB_CYAN | TB_BOLD, TB_BLACK);  // ╗
    y++;

    // ═══════════════════════════════════════════════════════════════════════
    // Title row
    // ═══════════════════════════════════════════════════════════════════════
    draw_left_border(y);
    print_left(y, 2, "MSX Tape Player", COLOR_TITLE);
    draw_middle_border(y);
    print_right(y, SPLIT_COL + 2, "Recent Activity", COLOR_TITLE);
    draw_right_border(y);
    y++;

    // ═══════════════════════════════════════════════════════════════════════
    // Divider after titles
    // ═══════════════════════════════════════════════════════════════════════
    draw_full_separator(y, TB_CYAN | TB_BOLD);
    y++;

    int content_start_y = y;

    // ═══════════════════════════════════════════════════════════════════════
    // LEFT PANEL: Player information
    // ═══════════════════════════════════════════════════════════════════════

    // Status
    draw_left_border(y);
    const char *status = isPlaying(player) ? "\xE2\x96\xB6 Playing" :
                        isPaused(player) ? "\xE2\x8F\xB8 Paused" : "\xE2\x8F\xB9 Stopped";
    printf_left(y, 2, COLOR_VALUE, "Status: %s", status);
    y++;

    // Volume
    draw_left_border(y);
    printf_left(y, 2, COLOR_VALUE, "Volume: %.0f%%", player->volume * 100);
    y++;

    // Empty line
    draw_left_empty_line(y);
    y++;

    // Tape time (current / remaining / total)
    draw_left_border(y);
    double remaining = total - current;
    if (remaining < 0) remaining = 0;
    printf_left(y, 2, COLOR_VALUE, "Tape:   %02d:%02d / %02d:%02d / %02d:%02d",
                (int)(current / 60), (int)current % 60,
                (int)(remaining / 60), (int)remaining % 60,
                (int)(total / 60), (int)total % 60);
    y++;

    // Tape progress bar
    draw_left_progress(y, current, total);
    y++;

    // Separator
    draw_left_separator(y);
    y++;

    // Current activity (what's happening right now - sync, silence, etc.)
    const char *activity_value = state->current_activity ? 
        strip_marker_prefix(state->current_activity->description) : "(waiting...)";
    uint32_t activity_color = state->current_activity ? COLOR_ACTIVITY : COLOR_DIM;
    draw_left_label_value(y, "Now: ", activity_value, activity_color);
    y++;

    // File name
    draw_left_border(y);
    print_left(y, 2, "File:", COLOR_LABEL);
    if (state->current_file) {
        const char *desc = state->current_file->description;
        const char *file_start = strstr(desc, "File ");
        if (file_start) {
            print_left(y, 8, file_start, COLOR_INFO);
        } else {
            fill_line(y, 8, SPLIT_COL);
        }
    } else {
        print_left(y, 8, "(no file)", COLOR_DIM);
    }
    y++;

    // Blank line for visual separation
    draw_left_empty_line(y);
    y++;

    // Block type
    draw_left_border(y);
    print_left(y, 2, "Data:", COLOR_LABEL);
    if (state->current_block) {
        // Remove "[STRUCTURE] " prefix if present
        const char *desc = state->current_block->description;
        const char *clean_desc = strstr(desc, "] ");
        if (clean_desc) clean_desc += 2;  // Skip "] "
        else clean_desc = desc;

        // Remove "Data " prefix (e.g., "Data block 1/1" -> "block 1/1")
        if (strncmp(clean_desc, "Data ", 5) == 0) {
            clean_desc += 5;
        }

        print_left(y, 8, clean_desc, COLOR_VALUE);
    } else {
        print_left(y, 8, "(no data)", COLOR_DIM);
    }
    y++;

    // Block progress bar
    // Calculate block progress based on current position within block
    double block_current = 0.0;
    double block_total = 1.0;
    if (state->current_block && markers) {
        double block_start = state->current_block->time_seconds;
        // Find next marker (any type) to get block end time
        // The block ends when we hit silence/sync after the data
        double block_end = current;
        for (size_t i = 0; i < markers->count; i++) {
            if (markers->markers[i].time_seconds > block_start) {
                block_end = markers->markers[i].time_seconds;
                break;
            }
        }
        if (block_end > block_start) {
            block_total = block_end - block_start;
            block_current = current - block_start;
            if (block_current < 0) block_current = 0;
            // If we've passed the block end, reset to 0 instead of staying at 100%
            if (block_current > block_total) {
                block_current = 0;
                block_total = 1.0;  // Reset to show empty bar
            }
        }
    }
    draw_left_progress(y, block_current, block_total);
    y++;

    // Separator
    draw_left_separator(y);
    y++;

    // Static info: Tape summary
    draw_left_border(y);
    if (markers) {
        int file_count = 0;
        for (size_t i = 0; i < markers->count; i++) {
            if (markers->markers[i].category == MARKER_STRUCTURE) {
                const char *desc = markers->markers[i].description;
                if (strstr(desc, "File ") && strstr(desc, "/") && strstr(desc, ":")) {
                    const char *slash = strstr(desc, "/");
                    if (slash) {
                        sscanf(slash + 1, "%d", &file_count);
                    }
                }
            }
        }
        printf_left(y, 2, COLOR_INFO, "Tape: %d files \xE2\x80\xA2 1200 bps", file_count);
    } else {
        print_left(y, 2, "Mode: Basic audio playback", COLOR_INFO);
    }
    y++;

    // Static info: Audio format
    draw_left_border(y);
    printf_left(y, 2, COLOR_INFO, "Audio: 44.1kHz Mono \xE2\x80\xA2 %zu markers",
                markers ? markers->count : 0);
    y++;

    // Static info: Filename
    draw_left_border(y);
    const char *filepath = player->filepath ? player->filepath : "unknown";
    const char *basename = strrchr(filepath, '/');
    if (basename) basename++;
    else basename = filepath;
    printf_left(y, 2, COLOR_INFO, "File: %s", basename);
    y++;

    int left_end_y = y;

    // ═══════════════════════════════════════════════════════════════════════
    // RIGHT PANEL: Recent activity
    // ═══════════════════════════════════════════════════════════════════════

    int right_y = content_start_y;

    // Show up to MAX_ACTIVITIES recent events
    for (int i = 0; i < MAX_ACTIVITIES; i++) {
        draw_middle_border(right_y);

        if (i < state->detail_count) {
            RecentMarker *m = &state->detail_markers[i];
            const char *clean_desc = strip_marker_prefix(m->description);
            char buf[128];
            snprintf(buf, sizeof(buf), "[%6.2fs] %s", m->time, clean_desc);
            print_right(right_y, SPLIT_COL + 2, buf, COLOR_VALUE);
        } else if (i == 0 && state->detail_count == 0) {
            const char *msg = markers ? "(none)" : "(no markers - basic playback)";
            print_right(right_y, SPLIT_COL + 2, msg, COLOR_DIM);
        } else {
            fill_line(right_y, SPLIT_COL + 1, TOTAL_WIDTH - 1);
        }

        draw_right_border(right_y);
        right_y++;
    }

    // Sync left and right panel heights
    while (left_end_y < right_y) {
        draw_left_border(left_end_y);
        fill_line(left_end_y, 1, SPLIT_COL);
        left_end_y++;
    }

    y = (left_end_y > right_y) ? left_end_y : right_y;

    // ═══════════════════════════════════════════════════════════════════════
    // Bottom section: Help or hint
    // ═══════════════════════════════════════════════════════════════════════

    // Divider
    tb_set_cell(0, y, 0x2560, COLOR_BORDER, TB_BLACK);  // ╠
    draw_hline(y, 1, SPLIT_COL, 0x2550, COLOR_BORDER);  // ═
    tb_set_cell(SPLIT_COL, y, 0x2569, COLOR_BORDER, TB_BLACK);  // ╩
    draw_hline(y, SPLIT_COL + 1, TOTAL_WIDTH - 1, 0x2550, COLOR_BORDER);  // ═
    tb_set_cell(TOTAL_WIDTH - 1, y, 0x2563, COLOR_BORDER, TB_BLACK);  // ╣
    y++;

    // Help text or hint
    draw_left_border(y);
    print_left(y, 2, "Press 'h' for help", COLOR_DIM);
    draw_middle_border(y);
    print_right_aligned(y, "github.com/xesco · © 2026", COLOR_DIM);
    draw_right_border(y);
    y++;

    // Bottom border
    tb_set_cell(0, y, 0x255A, COLOR_BORDER, TB_BLACK);  // ╚
    draw_hline(y, 1, TOTAL_WIDTH - 1, 0x2550, COLOR_BORDER);  // ═
    tb_set_cell(TOTAL_WIDTH - 1, y, 0x255D, COLOR_BORDER, TB_BLACK);  // ╝

    tb_present();
}

// =============================================================================
// Main Play Function
// =============================================================================

int execute_play(const char *filename, bool verbose) {
    (void)verbose;

    // Create audio player
    AudioPlayer *player = createAudioPlayer(filename);
    if (!player) {
        fprintf(stderr, "Error: Failed to create audio player\n");
        return 1;
    }

    // Initialize termbox
    int ret = tb_init();
    if (ret != 0) {
        fprintf(stderr, "Error: Failed to initialize terminal (code %d)\n", ret);
        destroyAudioPlayer(player);
        return 1;
    }

    tb_set_input_mode(TB_INPUT_ESC);
    tb_set_output_mode(TB_OUTPUT_NORMAL);
    tb_hide_cursor();

    // Read markers (optional - works without them)
    MarkerListInfo *markers = readWavMarkers(filename);

    // Initialize display state
    DisplayState state = {0};

    // Start playback
    playAudio(player);

    bool running = true;
    bool show_help = false;

    // Main event loop
    while (running) {
        // Update display state
        double current_time = getPlaybackPosition(player);
        updateDisplayState(&state, markers, current_time);

        // Render display
        renderDisplay(player, &state, markers, show_help);

        // Check for input (non-blocking with 50ms timeout)
        struct tb_event ev;
        int poll_ret = tb_peek_event(&ev, 50);

        if (poll_ret == 0 && ev.type == TB_EVENT_KEY) {
            // Handle key events
            if (ev.key == TB_KEY_ESC || ev.ch == 'q' || ev.ch == 'Q') {
                running = false;
            } else if (ev.ch == 'h' || ev.ch == 'H') {
                show_help = !show_help;
            } else if (!show_help) {
                // Only handle playback controls when help is not shown
                if (ev.ch == ' ') {
                    if (isPlaying(player)) {
                        pauseAudio(player);
                    } else {
                        playAudio(player);
                    }
                } else if (ev.key == TB_KEY_ARROW_UP) {
                    setVolume(player, player->volume + 0.1);
                } else if (ev.key == TB_KEY_ARROW_DOWN) {
                    setVolume(player, player->volume - 0.1);
                } else if (ev.key == TB_KEY_ARROW_RIGHT) {
                    seekAudio(player, current_time + 5.0);
                } else if (ev.key == TB_KEY_ARROW_LEFT) {
                    double new_pos = current_time - 5.0;
                    if (new_pos < 0.0) new_pos = 0.0;
                    seekAudio(player, new_pos);
                }
            }
        }

        // Check if playback ended
        if (isPlaying(player) && current_time >= player->total_duration) {
            running = false;
        }
    }

    // Cleanup
    pauseAudio(player);
    tb_shutdown();

    if (markers) {
        free(markers->markers);
        free(markers);
    }

    destroyAudioPlayer(player);

    return 0;
}
