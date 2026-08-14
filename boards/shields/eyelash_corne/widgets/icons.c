/*
 *
 * Copyright (c) 2026 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <math.h>
#include "icons.h"
#include "status.h"

#define EC_PI 3.14159265f
#define LINE_W 2

/* Points are authored on a 20x20 design grid and scaled to the requested
 * icon size at draw time, so one point list works at any call site. */
static inline lv_coord_t sx(int dx, int size, float v) {
    return (lv_coord_t)(dx + v * size / 20.0f);
}
static inline lv_coord_t sy(int dy, int size, float v) {
    return (lv_coord_t)(dy + v * size / 20.0f);
}

static void draw_path(lv_obj_t *canvas, int dx, int dy, int size, const float pts[][2], int n) {
    lv_draw_line_dsc_t dsc;
    init_line_dsc(&dsc, LVGL_FOREGROUND, LINE_W);
    lv_point_t scaled[8];
    for (int i = 0; i < n; i++) {
        scaled[i].x = sx(dx, size, pts[i][0]);
        scaled[i].y = sy(dy, size, pts[i][1]);
    }
    lv_canvas_draw_line(canvas, scaled, n, &dsc);
}

/* Base: home */
void ec_icon_home(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float roof[][2] = {{3, 10}, {10, 3}, {17, 10}};
    static const float body[][2] = {{6, 10}, {6, 17}, {14, 17}, {14, 10}, {6, 10}};
    draw_path(canvas, dx, dy, size, roof, 3);
    draw_path(canvas, dx, dy, size, body, 5);

    lv_draw_rect_dsc_t door;
    init_rect_dsc(&door, LVGL_FOREGROUND);
    lv_coord_t x = sx(dx, size, 9), y = sy(dy, size, 12);
    lv_coord_t w = (lv_coord_t)(2 * size / 20.0f), h = (lv_coord_t)(5 * size / 20.0f);
    lv_canvas_draw_rect(canvas, x, y, w > 0 ? w : 1, h > 0 ? h : 1, &door);
}

/* Nav: four-way d-pad arrows */
void ec_icon_dpad(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float up[][2] = {{7, 7}, {10, 2}, {13, 7}};
    static const float down[][2] = {{7, 13}, {10, 18}, {13, 13}};
    static const float left[][2] = {{7, 7}, {2, 10}, {7, 13}};
    static const float right[][2] = {{13, 7}, {18, 10}, {13, 13}};
    draw_path(canvas, dx, dy, size, up, 3);
    draw_path(canvas, dx, dy, size, down, 3);
    draw_path(canvas, dx, dy, size, left, 3);
    draw_path(canvas, dx, dy, size, right, 3);
}

/* Num: hash */
void ec_icon_hash(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float v1[][2] = {{7, 3}, {7, 17}};
    static const float v2[][2] = {{13, 3}, {13, 17}};
    static const float h1[][2] = {{3, 7}, {17, 7}};
    static const float h2[][2] = {{3, 13}, {17, 13}};
    draw_path(canvas, dx, dy, size, v1, 2);
    draw_path(canvas, dx, dy, size, v2, 2);
    draw_path(canvas, dx, dy, size, h1, 2);
    draw_path(canvas, dx, dy, size, h2, 2);
}

/* Sym: starburst */
void ec_icon_starburst(lv_obj_t *canvas, int dx, int dy, int size) {
    lv_draw_line_dsc_t dsc;
    init_line_dsc(&dsc, LVGL_FOREGROUND, LINE_W);
    for (int a = 0; a < 360; a += 45) {
        float rad = a * EC_PI / 180.0f;
        float x1 = 10 + 3 * cosf(rad), y1 = 10 + 3 * sinf(rad);
        float x2 = 10 + 9 * cosf(rad), y2 = 10 + 9 * sinf(rad);
        lv_point_t pts[2] = {
            {sx(dx, size, x1), sy(dy, size, y1)},
            {sx(dx, size, x2), sy(dy, size, y2)},
        };
        lv_canvas_draw_line(canvas, pts, 2, &dsc);
    }
}

/* Media: play triangle */
void ec_icon_play(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float tri[][2] = {{7, 4}, {7, 16}, {16, 10}, {7, 4}};
    draw_path(canvas, dx, dy, size, tri, 4);
}

/* Fun: gear */
void ec_icon_gear(lv_obj_t *canvas, int dx, int dy, int size) {
    lv_coord_t cx = sx(dx, size, 10), cy = sy(dy, size, 10);
    lv_coord_t r = (lv_coord_t)(5 * size / 20.0f);

    lv_draw_arc_dsc_t ring;
    init_arc_dsc(&ring, LVGL_FOREGROUND, LINE_W);
    lv_canvas_draw_arc(canvas, cx, cy, r, 0, 360, &ring);

    lv_coord_t hub_r = (lv_coord_t)(1.6f * size / 20.0f);
    if (hub_r < 1) {
        hub_r = 1;
    }
    lv_draw_arc_dsc_t hub;
    init_arc_dsc(&hub, LVGL_FOREGROUND, hub_r);
    lv_canvas_draw_arc(canvas, cx, cy, hub_r, 0, 360, &hub);

    lv_draw_line_dsc_t tick;
    init_line_dsc(&tick, LVGL_FOREGROUND, LINE_W);
    for (int a = 0; a < 360; a += 60) {
        float rad = a * EC_PI / 180.0f;
        float x1 = 10 + 5 * cosf(rad), y1 = 10 + 5 * sinf(rad);
        float x2 = 10 + 8 * cosf(rad), y2 = 10 + 8 * sinf(rad);
        lv_point_t pts[2] = {
            {sx(dx, size, x1), sy(dy, size, y1)},
            {sx(dx, size, x2), sy(dy, size, y2)},
        };
        lv_canvas_draw_line(canvas, pts, 2, &tick);
    }
}

/* Mouse: device silhouette */
void ec_icon_mouse(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float body[][2] = {{6, 3}, {14, 3}, {14, 17}, {6, 17}, {6, 3}};
    static const float divider[][2] = {{10, 3}, {10, 8}};
    draw_path(canvas, dx, dy, size, body, 5);
    draw_path(canvas, dx, dy, size, divider, 2);

    lv_draw_rect_dsc_t notch;
    init_rect_dsc(&notch, LVGL_FOREGROUND);
    lv_coord_t x = sx(dx, size, 9), y = sy(dy, size, 5);
    lv_coord_t w = (lv_coord_t)(2 * size / 20.0f), h = (lv_coord_t)(3 * size / 20.0f);
    lv_canvas_draw_rect(canvas, x, y, w > 0 ? w : 1, h > 0 ? h : 1, &notch);
}

/* Button: cursor with click burst */
void ec_icon_cursor_click(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float cursor[][2] = {
        {5, 4}, {5, 16}, {9, 12.5f}, {11.5f, 17}, {13.5f, 16}, {11, 11.5f}, {15, 11}, {5, 4},
    };
    draw_path(canvas, dx, dy, size, cursor, 8);
}

/* Output row: bluetooth glyph */
void ec_icon_bluetooth(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float pts[][2] = {
        {10, 3}, {14, 7}, {6, 13}, {10, 17}, {6, 7}, {14, 13}, {10, 3},
    };
    draw_path(canvas, dx, dy, size, pts, 7);
}

/* Battery row: charging bolt */
void ec_icon_bolt(lv_obj_t *canvas, int dx, int dy, int size) {
    static const float pts[][2] = {
        {11, 1}, {4, 12}, {9, 12}, {7, 19}, {16, 7}, {11, 7}, {11, 1},
    };
    draw_path(canvas, dx, dy, size, pts, 7);
}

/* Peripheral link status: overlapping rings = linked to central */
void ec_icon_link_ok(lv_obj_t *canvas, int dx, int dy, int size) {
    lv_draw_arc_dsc_t ring;
    init_arc_dsc(&ring, LVGL_FOREGROUND, LINE_W);
    lv_coord_t r = (lv_coord_t)(4 * size / 20.0f);
    lv_canvas_draw_arc(canvas, sx(dx, size, 7), sy(dy, size, 10), r, 0, 360, &ring);
    lv_canvas_draw_arc(canvas, sx(dx, size, 13), sy(dy, size, 10), r, 0, 360, &ring);
}

/* Peripheral link status: separated rings = not linked */
void ec_icon_link_broken(lv_obj_t *canvas, int dx, int dy, int size) {
    lv_draw_arc_dsc_t ring;
    init_arc_dsc(&ring, LVGL_FOREGROUND, LINE_W);
    lv_coord_t r = (lv_coord_t)(3 * size / 20.0f);
    lv_canvas_draw_arc(canvas, sx(dx, size, 5), sy(dy, size, 10), r, 0, 360, &ring);
    lv_canvas_draw_arc(canvas, sx(dx, size, 15), sy(dy, size, 10), r, 0, 360, &ring);
}

const ec_icon_fn EC_LAYER_ICONS[EC_LAYER_ICON_COUNT] = {
    ec_icon_home,         /* 0 Base */
    ec_icon_dpad,         /* 1 Nav */
    ec_icon_hash,         /* 2 Num */
    ec_icon_starburst,    /* 3 Sym */
    ec_icon_play,         /* 4 Media */
    ec_icon_gear,         /* 5 Fun */
    ec_icon_mouse,        /* 6 Mouse */
    ec_icon_cursor_click, /* 7 Button */
};
