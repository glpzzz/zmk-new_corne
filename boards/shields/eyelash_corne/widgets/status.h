/*
 *
 * Copyright (c) 2026 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zmk/endpoints.h>

/*
 * The physical SSD1306 panel is a 128x32 landscape strip, but it's mounted
 * rotated 90 degrees in the case, so content is drawn upright into a
 * 32(w)x128(h) logical canvas, then rotated into the real 128x32 canvas that
 * actually gets flushed to the panel. See ec_rotate_into_screen() in util.c.
 */
#define EC_CONTENT_W 32
#define EC_CONTENT_H 128
#define EC_SCREEN_W 128
#define EC_SCREEN_H 32

#define LVGL_BACKGROUND lv_color_black()
#define LVGL_FOREGROUND lv_color_white()

struct ec_status_state {
    uint8_t battery;
    bool charging;
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    uint8_t layer_index;
    const char *layer_label;
#else
    bool connected;
#endif
};

struct zmk_widget_status {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *content_canvas;
    lv_obj_t *screen_canvas;
    lv_color_t content_buf[EC_CONTENT_W * EC_CONTENT_H];
    lv_color_t screen_buf[EC_SCREEN_W * EC_SCREEN_H];
    struct ec_status_state state;
};

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget);

void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color);
void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                     lv_text_align_t align);
void ec_rotate_into_screen(lv_color_t *content_buf, lv_color_t *screen_buf);
