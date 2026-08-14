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
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include "icons.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_status_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif
};

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
};

struct layer_status_state {
    zmk_keymap_layer_index_t index;
    const char *label;
};

static void ec_redraw(struct zmk_widget_status *widget) {
    const struct ec_status_state *state = &widget->state;

    lv_draw_rect_dsc_t bg_dsc;
    init_rect_dsc(&bg_dsc, LVGL_BACKGROUND);
    lv_canvas_draw_rect(widget->content_canvas, 0, 0, EC_CONTENT_W, EC_CONTENT_H, &bg_dsc);

    /* Battery row: "53%" right-aligned, charging bolt icon to its right. */
    lv_draw_label_dsc_t label_dsc_right;
    init_label_dsc(&label_dsc_right, LVGL_FOREGROUND, &lv_font_montserrat_14, LV_TEXT_ALIGN_RIGHT);
    char batt_text[6] = {};
    snprintf(batt_text, sizeof(batt_text), "%d%%", state->battery);
    lv_canvas_draw_text(widget->content_canvas, 0, 10, 20, &label_dsc_right, batt_text);
    if (state->charging) {
        ec_icon_bolt(widget->content_canvas, 22, 14, 10);
    }

    /* Output row: bluetooth icon + profile number, or "USB". */
    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB: {
        lv_draw_label_dsc_t label_dsc_center;
        init_label_dsc(&label_dsc_center, LVGL_FOREGROUND, &lv_font_montserrat_14,
                        LV_TEXT_ALIGN_CENTER);
        lv_canvas_draw_text(widget->content_canvas, 0, 52, EC_CONTENT_W, &label_dsc_center, "USB");
        break;
    }
    case ZMK_TRANSPORT_BLE: {
        ec_icon_bluetooth(widget->content_canvas, 6, 48, 16);
        lv_draw_label_dsc_t label_dsc_left;
        init_label_dsc(&label_dsc_left, LVGL_FOREGROUND, &lv_font_montserrat_14,
                        LV_TEXT_ALIGN_LEFT);
        char profile_text[3] = {};
        snprintf(profile_text, sizeof(profile_text), "%d", state->active_profile_index + 1);
        lv_canvas_draw_text(widget->content_canvas, 24, 50, 8, &label_dsc_left, profile_text);
        break;
    }
    }

    /* Layer row: one icon per layer (see icons.c / EC_LAYER_ICONS). */
    if (state->layer_index < EC_LAYER_ICON_COUNT) {
        EC_LAYER_ICONS[state->layer_index](widget->content_canvas, 6, 94, 20);
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
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_status_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
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

static void set_output_status(struct zmk_widget_status *widget,
                               const struct output_status_state *state) {
    widget->state.selected_endpoint = state->selected_endpoint;
    widget->state.active_profile_index = state->active_profile_index;
    widget->state.active_profile_connected = state->active_profile_connected;
    widget->state.active_profile_bonded = state->active_profile_bonded;

    ec_redraw(widget);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget, &state); }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){
        .selected_endpoint = zmk_endpoints_selected(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                             output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

static void set_layer_status(struct zmk_widget_status *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;

    ec_redraw(widget);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){
        .index = index, .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index))};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                             layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

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
    widget_output_status_init();
    widget_layer_status_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
