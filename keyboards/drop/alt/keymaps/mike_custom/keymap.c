// Copyright 2023 Massdrop, Inc.
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

// MIKE: copied from "default"
// after git clone:
// make git-submodule
// qmk compile -kb massdrop/alt -km mac_mike_custom2
// fn-b for 3+ seconds to put keyboard into flash mode
// use QMK Toolbox or run:
// mdloader -p /dev/cu.usbmodem1113201 -D /Users/mrheinheimer/work/go/src/bitbucket.org/asecurityteam/qmk_firmware/massdrop_alt_mac_mike_custom2.bin --restart

// Some helpful docs:
// https://www.overclockersclub.com/reviews/cmk_qmk_via/8.htm

enum my_keycodes {
  LOCK_SCN = SAFE_RANGE, //Lock screen on a MacBook Pro
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_65_ansi_blocker(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, LOCK_SCN,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGDN,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_HOME,
        KC_LCTL, KC_LALT, KC_LGUI,                            KC_SPC,                             KC_RALT, MO(1),   KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [1] = LAYOUT_65_ansi_blocker(
        KC_GRV,  KC_F1,   KC_F2,    KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, KC_MUTE,
        _______, RM_TOGG, RM_VALU,  RM_SPDU, RM_HUEU, RM_SATU, _______, _______, _______, _______, KC_PSCR, KC_SCRL, KC_PAUS, _______, KC_VOLU, 
        _______, RM_NEXT, RM_VALD,  RM_SPDD, RM_HUED, RM_SATD, _______, _______, _______, _______, _______, _______,          _______, KC_VOLD,
        _______, RGB_M_P, RGB_M_TW, _______, _______, QK_BOOT, NK_TOGG, _______, _______, _______, _______, _______,          KC_PGUP, KC_END,
        _______, _______, _______,                             EE_CLR,                             KC_APP,  _______, KC_HOME, KC_PGDN, KC_END
    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    switch (keycode) {
        case LOCK_SCN:
            if (record->event.pressed) {
                SEND_STRING(SS_DOWN(X_LGUI) SS_DOWN(X_LCTRL) SS_TAP(X_Q) SS_UP(X_LCTRL) SS_UP(X_LGUI));
            }
            return false;
        default:
            return true; //Process all other keycodes normally
    }
}
