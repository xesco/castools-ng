/*
 * uilib.c - Terminal UI drawing library implementation
 */

#define TB_IMPL
#include "uilib.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// =============================================================================
// Basic Drawing Primitives
// =============================================================================

void draw_hline(int y, int start_x, int end_x, uint32_t ch, uintattr_t fg) {
    for (int x = start_x; x < end_x; x++) {
        tb_set_cell(x, y, ch, fg, TB_BLACK);
    }
}

void fill_line(int y, int start_x, int end_x) {
    for (int x = start_x; x < end_x; x++) {
        tb_set_cell(x, y, ' ', TB_WHITE, TB_BLACK);
    }
}

void draw_left_border(int y) {
    tb_set_cell(0, y, 0x2551, COLOR_BORDER, TB_BLACK);  // ║
}

void draw_middle_border(int y) {
    tb_set_cell(SPLIT_COL, y, 0x2551, COLOR_BORDER, TB_BLACK);  // ║
}

void draw_right_border(int y) {
    tb_set_cell(TOTAL_WIDTH - 1, y, 0x2551, COLOR_BORDER, TB_BLACK);  // ║
}

void draw_row_borders(int y) {
    draw_left_border(y);
    draw_middle_border(y);
    draw_right_border(y);
}

void print_left(int y, int x, const char *text, uintattr_t fg) {
    tb_print(x, y, fg, TB_BLACK, text);
    int len = strlen(text);
    fill_line(y, x + len, SPLIT_COL);
}

void printf_left(int y, int x, uintattr_t fg, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    tb_print(x, y, fg, TB_BLACK, buf);
    int len = strlen(buf);
    fill_line(y, x + len, SPLIT_COL);
}

void print_right(int y, int x, const char *text, uintattr_t fg) {
    int max_len = TOTAL_WIDTH - 1 - x;
    char buf[256];
    snprintf(buf, sizeof(buf), "%.*s", max_len, text);
    tb_print(x, y, fg, TB_BLACK, buf);
    int len = strlen(buf);
    fill_line(y, x + len, TOTAL_WIDTH - 1);
}

void printf_right(int y, int x, uintattr_t fg, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    print_right(y, x, buf, fg);
}

// =============================================================================
// Box Drawing
// =============================================================================

void draw_box_top(int y, int left, int right, uint32_t style) {
    tb_set_cell(left, y, 0x2554, style, TB_BLACK);  // ╔
    draw_hline(y, left + 1, right, 0x2550, style);  // ═
    tb_set_cell(right, y, 0x2557, style, TB_BLACK);  // ╗
}

void draw_box_bottom(int y, int left, int right, uint32_t style) {
    tb_set_cell(left, y, 0x255A, style, TB_BLACK);  // ╚
    draw_hline(y, left + 1, right, 0x2550, style);  // ═
    tb_set_cell(right, y, 0x255D, style, TB_BLACK);  // ╝
}

void draw_box_separator(int y, int left, int right, uint32_t style) {
    tb_set_cell(left, y, 0x2560, style, TB_BLACK);  // ╠
    draw_hline(y, left + 1, right, 0x2550, style);  // ═
    tb_set_cell(right, y, 0x2563, style, TB_BLACK);  // ╣
}

void draw_box_line(int y, int left, int right, const char *text, uint32_t text_color) {
    tb_set_cell(left, y, 0x2551, COLOR_BORDER, TB_BLACK);  // ║
    if (text) {
        tb_print(left + 2, y, text_color, TB_BLACK, text);
        for (int x = left + 2 + strlen(text); x < right; x++) {
            tb_set_cell(x, y, ' ', text_color, TB_BLACK);
        }
    } else {
        for (int x = left + 1; x < right; x++) {
            tb_set_cell(x, y, ' ', TB_WHITE, TB_BLACK);
        }
    }
    tb_set_cell(right, y, 0x2551, COLOR_BORDER, TB_BLACK);  // ║
}

// =============================================================================
// Left Panel Helpers
// =============================================================================

void draw_left_empty_line(int y) {
    draw_left_border(y);
    fill_line(y, 1, SPLIT_COL);
}

void draw_left_label_value(int y, const char *label, const char *value, uint32_t value_color) {
    draw_left_border(y);
    print_left(y, 2, label, COLOR_LABEL);
    int value_x = 2 + strlen(label) + 1;
    if (value) {
        print_left(y, value_x, value, value_color);
    } else {
        fill_line(y, value_x, SPLIT_COL);
    }
}

void draw_left_separator(int y) {
    draw_left_border(y);
    draw_hline(y, 1, SPLIT_COL, 0x2500, COLOR_SEPARATOR);  // ─
}

void draw_left_progress(int y, double current, double total) {
    draw_left_border(y);
    draw_progress_bar(y, 2, SPLIT_COL - 1, current, total);
}

// =============================================================================
// Full-Width Helpers
// =============================================================================

void draw_full_separator(int y, uint32_t style) {
    tb_set_cell(0, y, 0x2560, style, TB_BLACK);  // ╠
    draw_hline(y, 1, SPLIT_COL, 0x2550, style);  // ═
    tb_set_cell(SPLIT_COL, y, 0x256C, style, TB_BLACK);  // ╬
    draw_hline(y, SPLIT_COL + 1, TOTAL_WIDTH - 1, 0x2550, style);  // ═
    tb_set_cell(TOTAL_WIDTH - 1, y, 0x2563, style, TB_BLACK);  // ╣
}

// =============================================================================
// Specialized Components
// =============================================================================

void draw_progress_bar(int y, int start_x, int end_x, double current, double total) {
    int bar_width = end_x - start_x - 10;  // Reserve space for "[ ] XX.X%"
    if (bar_width < 5) return;
    if (total <= 0) total = 1.0;  // Avoid division by zero
    if (current < 0) current = 0;
    if (current > total) current = total;

    double percentage = (current / total * 100.0);
    int filled = (int)((current / total) * bar_width);
    if (filled > bar_width) filled = bar_width;
    if (filled < 0) filled = 0;

    // Draw progress bar
    tb_set_cell(start_x, y, '[', TB_WHITE, TB_BLACK);
    for (int i = 0; i < bar_width; i++) {
        char ch = ' ';
        if (i < filled) ch = '=';
        else if (i == filled) ch = '>';
        tb_set_cell(start_x + 1 + i, y, ch, COLOR_PROGRESS, TB_BLACK);
    }
    tb_set_cell(start_x + 1 + bar_width, y, ']', TB_WHITE, TB_BLACK);
    tb_printf(start_x + 2 + bar_width, y, TB_WHITE, TB_BLACK, " %.1f%%", percentage);
}

// =============================================================================
// Utility Functions
// =============================================================================

const char* strip_marker_prefix(const char *desc) {
    const char *clean = strstr(desc, "] ");
    return clean ? clean + 2 : desc;
}
