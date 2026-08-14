/*
 *
 * Copyright (c) 2026 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/display.h>
#include "status.h"
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/split/bluetooth/peripheral.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/usb.h>
#include "icons.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_status_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif
};

struct peripheral_conn_state {
    bool connected;
};

static void ec_redraw(struct zmk_widget_status *widget) {
    const struct ec_status_state *state = &widget->state;

    lv_draw_rect_dsc_t bg_dsc;
    init_rect_dsc(&bg_dsc, LVGL_BACKGROUND);
    lv_canvas_draw_rect(widget->content_canvas, 0, 0, EC_CONTENT_W, EC_CONTENT_H, &bg_dsc);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);

    /* Battery band, y 0-64: percentage full-width, bolt icon stacked below
     * when charging. */
    char batt_text[6] = {};
    snprintf(batt_text, sizeof(batt_text), "%d%%", state->battery);
    lv_canvas_draw_text(widget->content_canvas, 0, 16, EC_CONTENT_W, &label_dsc, batt_text);
    if (state->charging) {
        ec_icon_bolt(widget->content_canvas, 9, 36, 14);
    }

    /* Link band, y 64-128: overlapping rings = linked to central, separated
     * = not. */
    if (state->connected) {
        ec_icon_link_ok(widget->content_canvas, 6, 92, 20);
    } else {
        ec_icon_link_broken(widget->content_canvas, 6, 92, 20);
    }

    ec_rotate_into_screen(widget->content_buf, widget->screen_buf);
    lv_obj_invalidate(widget->screen_canvas);
}

static void set_battery_status(struct zmk_widget_status *widget,
                                struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

    widget->state.battery = state.level;

    ec_redraw(widget);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    return (struct battery_status_state){
        .level = zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                             battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

static struct peripheral_conn_state peripheral_conn_get_state(const zmk_event_t *_eh) {
    return (struct peripheral_conn_state){.connected = zmk_split_bt_peripheral_is_connected()};
}

static void set_connection_status(struct zmk_widget_status *widget,
                                   struct peripheral_conn_state state) {
    widget->state.connected = state.connected;

    ec_redraw(widget);
}

static void peripheral_conn_update_cb(struct peripheral_conn_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_connection_status(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_conn, struct peripheral_conn_state,
                             peripheral_conn_update_cb, peripheral_conn_get_state)
ZMK_SUBSCRIPTION(widget_peripheral_conn, zmk_split_peripheral_status_changed);

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, EC_SCREEN_W, EC_SCREEN_H);

    widget->content_canvas = lv_canvas_create(widget->obj);
    lv_canvas_set_buffer(widget->content_canvas, widget->content_buf, EC_CONTENT_W, EC_CONTENT_H,
                          LV_IMG_CF_TRUE_COLOR);
    lv_obj_add_flag(widget->content_canvas, LV_OBJ_FLAG_HIDDEN);

    widget->screen_canvas = lv_canvas_create(widget->obj);
    lv_canvas_set_buffer(widget->screen_canvas, widget->screen_buf, EC_SCREEN_W, EC_SCREEN_H,
                          LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(widget->screen_canvas, LV_ALIGN_TOP_LEFT, 0, 0);

    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();
    widget_peripheral_conn_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
