/*
 *
 * Copyright (c) 2026 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <lvgl.h>

/* All icons are drawn into `canvas` at native size x size pixels, top-left
 * corner at (dx, dy). Coordinates are authored in a 20x20 design grid and
 * scaled to `size` at draw time (see icons.c). */
typedef void (*ec_icon_fn)(lv_obj_t *canvas, int dx, int dy, int size);

void ec_icon_home(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_dpad(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_hash(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_starburst(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_play(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_gear(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_mouse(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_cursor_click(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_bluetooth(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_bolt(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_link_ok(lv_obj_t *canvas, int dx, int dy, int size);
void ec_icon_link_broken(lv_obj_t *canvas, int dx, int dy, int size);

/* Indexed by declaration order in eyelash_corne.keymap:
 * default_layer=0, nav_layer=1, num_layer=2, sym_layer=3, media_layer=4,
 * fun_layer=5, mouse_layer=6, button_layer=7. */
#define EC_LAYER_ICON_COUNT 8
extern const ec_icon_fn EC_LAYER_ICONS[EC_LAYER_ICON_COUNT];
