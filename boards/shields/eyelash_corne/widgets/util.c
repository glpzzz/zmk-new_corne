/*
 *
 * Copyright (c) 2026 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>
#include "status.h"

void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color) {
    lv_draw_rect_dsc_init(rect_dsc);
    rect_dsc->bg_color = bg_color;
}

void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                     lv_text_align_t align) {
    lv_draw_label_dsc_init(label_dsc);
    label_dsc->color = color;
    label_dsc->font = font;
    label_dsc->align = align;
}

void init_line_dsc(lv_draw_line_dsc_t *line_dsc, lv_color_t color, uint8_t width) {
    lv_draw_line_dsc_init(line_dsc);
    line_dsc->color = color;
    line_dsc->width = width;
}

void init_arc_dsc(lv_draw_arc_dsc_t *arc_dsc, lv_color_t color, uint8_t width) {
    lv_draw_arc_dsc_init(arc_dsc);
    arc_dsc->color = color;
    arc_dsc->width = width;
}

/* Rotate the upright 32x128 content buffer 90 degrees clockwise into the
 * physical 128x32 screen buffer. */
void ec_rotate_into_screen(lv_color_t *content_buf, lv_color_t *screen_buf) {
    for (int y = 0; y < EC_CONTENT_H; y++) {
        for (int x = 0; x < EC_CONTENT_W; x++) {
            int nx = EC_CONTENT_H - 1 - y;
            int ny = x;
            screen_buf[ny * EC_SCREEN_W + nx] = content_buf[y * EC_CONTENT_W + x];
        }
    }
}
