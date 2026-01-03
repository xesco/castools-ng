/*
 * uilib.h - Terminal UI drawing library for castools
 * 
 * Provides high-level drawing functions using termbox2 for
 * creating split-panel layouts, progress bars, boxes, etc.
 */

#ifndef UILIB_H
#define UILIB_H

#include "termbox2.h"

// =============================================================================
// Layout Constants
// =============================================================================

#define TOTAL_WIDTH  100
#define LEFT_WIDTH   50
#define RIGHT_WIDTH  48
#define SPLIT_COL    LEFT_WIDTH
#define MAX_ACTIVITIES 16

// =============================================================================
// Color Scheme
// =============================================================================

#define COLOR_BORDER    (TB_CYAN | TB_BOLD)
#define COLOR_TITLE     (TB_WHITE | TB_BOLD)
#define COLOR_LABEL     (TB_YELLOW)
#define COLOR_VALUE     TB_WHITE
#define COLOR_INFO      TB_CYAN
#define COLOR_ACTIVITY  TB_GREEN
#define COLOR_DIM       TB_HI_BLACK
#define COLOR_PROGRESS  TB_CYAN
#define COLOR_SEPARATOR TB_BLUE

// =============================================================================
// Basic Drawing Primitives
// =============================================================================

void draw_hline(int y, int start_x, int end_x, uint32_t ch, uintattr_t fg);
void fill_line(int y, int start_x, int end_x);

// Border drawing
void draw_left_border(int y);
void draw_middle_border(int y);
void draw_right_border(int y);
void draw_row_borders(int y);

// Text printing
void print_left(int y, int x, const char *text, uintattr_t fg);
void printf_left(int y, int x, uintattr_t fg, const char *fmt, ...);
void print_right(int y, int x, const char *text, uintattr_t fg);
void printf_right(int y, int x, uintattr_t fg, const char *fmt, ...);
void print_right_aligned(int y, const char *text, uintattr_t fg);

// =============================================================================
// Box Drawing
// =============================================================================

void draw_box_top(int y, int left, int right, uint32_t style);
void draw_box_bottom(int y, int left, int right, uint32_t style);
void draw_box_separator(int y, int left, int right, uint32_t style);
void draw_box_line(int y, int left, int right, const char *text, uint32_t text_color);

// =============================================================================
// Left Panel Helpers
// =============================================================================

void draw_left_empty_line(int y);
void draw_left_label_value(int y, const char *label, const char *value, uint32_t value_color);
void draw_left_separator(int y);
void draw_left_progress(int y, double current, double total);

// =============================================================================
// Full-Width Helpers
// =============================================================================

void draw_full_separator(int y, uint32_t style);

// =============================================================================
// Specialized Components
// =============================================================================

void draw_progress_bar(int y, int start_x, int end_x, double current, double total);

// =============================================================================
// Utility Functions
// =============================================================================

const char* strip_marker_prefix(const char *desc);

#endif // UILIB_H
