#include QMK_KEYBOARD_H
#include "config.h"

// MIKE TODO: study and continue with porting this:
// https://www.bikuman.com/atomantium/qmk_firmware/commit/63e212c0b78be664785433c11cd728f15f50cd6a

enum alt_keycodes {
    L_BRI = SAFE_RANGE, //LED Brightness Increase                                   //Working
    L_BRD,              //LED Brightness Decrease                                   //Working
    L_PTN,              //LED Pattern Select Next                                   //Working
    L_PTP,              //LED Pattern Select Previous                               //Working
    L_PSI,              //LED Pattern Speed Increase                                //Working
    L_PSD,              //LED Pattern Speed Decrease                                //Working
    L_T_MD,             //LED Toggle Mode                                           //Working
    L_T_ONF,            //LED Toggle On / Off                                       //Broken
    L_ON,               //LED On                                                    //Broken
    L_OFF,              //LED Off                                                   //Broken
    L_T_BR,             //LED Toggle Breath Effect                                  //Working
    L_T_PTD,            //LED Toggle Scrolling Pattern Direction                    //Working
    U_T_AGCR,           //USB Toggle Automatic GCR control                          //Working
    DBG_TOG,            //DEBUG Toggle On / Off                                     //
    DBG_MTRX,           //DEBUG Toggle Matrix Prints                                //
    DBG_KBD,            //DEBUG Toggle Keyboard Prints                              //
    DBG_MOU,            //DEBUG Toggle Mouse Prints                                 //
    MD_BOOT,            //Restart into bootloader after hold timeout                //Working
    LOCK_SCN            //Lock screen on a MacBook Pro
};

#define TG_NKRO MAGIC_TOGGLE_NKRO //Toggle 6KRO / NKRO mode

#define LED_BOOST_PEAK 100
#define LED_BOOST_STEP_DOWN 5
#define LED_BOOST_REFRESH_INTERVAL_IN_MS 20

extern issi3733_led_t led_map[];

static uint16_t last_boost_update_timer;
static bool caps_lock_on;

keymap_config_t keymap_config;

static uint8_t led_boosts[ISSI3733_LED_COUNT];

// #define __ -1
// static const uint8_t KEY_TO_LED_MAP[MATRIX_ROWS][MATRIX_COLS] = {
//   { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14},
//   {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29},
//   {30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, __, 42, 43},
//   {44, __, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57},
//   {58, 59, 60, __, __, __, 61, __, __, __, 62, 63, 64, 65, 66},
// };

void maybe_do_fade_on_keypress(keyrecord_t *record);
static uint8_t map_key_position_to_led_index(uint8_t col, uint8_t row);
static void update_all_leds(void);
static void step_down_all_boosts(void);


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, LOCK_SCN,  \
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_HOME, \
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGUP, \
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_PGDN, \
        KC_LCTL, KC_LALT, KC_LGUI,                            KC_SPC,                             KC_RGUI, MO(1),   KC_LEFT, KC_DOWN, KC_RGHT  \
    ),
    [1] = LAYOUT(
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, _______, \
        L_T_BR,  L_PSD,   L_BRI,   L_PSI,   _______, _______, _______, _______, U_T_AGCR,_______, KC_PSCR, KC_SLCK, KC_PAUS, _______, KC_MUTE, \
        L_T_PTD, L_PTP,   L_BRD,   L_PTN,   _______, _______, _______, _______, _______, _______, _______, _______,          _______, KC_VOLU, \
        _______, L_T_MD,  L_T_ONF, _______, _______, MD_BOOT, TG_NKRO, _______, _______, _______, _______, _______,          KC_PGUP, KC_VOLD, \
        _______, _______, _______,                            _______,                            _______, _______, KC_HOME, KC_PGDN, KC_END  \
    ),
    /*
    [X] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, _______,                            _______,                            _______, _______, _______, _______, _______  \
    ),
    */
};

// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {
  caps_lock_on = false;
  last_boost_update_timer = timer_read32();
};

// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {
  if (led_enabled && led_animation_id == 0 && (timer_elapsed32(last_boost_update_timer) > LED_BOOST_REFRESH_INTERVAL_IN_MS)) {
    last_boost_update_timer = timer_read32();
    step_down_all_boosts();
  }
  if (led_enabled && led_animation_id == 0) {
    update_all_leds();
  }
};

#define MODS_SHIFT  (get_mods() & MOD_BIT(KC_LSHIFT) || get_mods() & MOD_BIT(KC_RSHIFT))
#define MODS_CTRL  (get_mods() & MOD_BIT(KC_LCTL) || get_mods() & MOD_BIT(KC_RCTRL))
#define MODS_ALT  (get_mods() & MOD_BIT(KC_LALT) || get_mods() & MOD_BIT(KC_RALT))

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    maybe_do_fade_on_keypress(record);

    switch (keycode) {
        case L_BRI:
            if (record->event.pressed) {
                if (LED_GCR_STEP > LED_GCR_MAX - gcr_desired) gcr_desired = LED_GCR_MAX;
                else gcr_desired += LED_GCR_STEP;
                if (led_animation_breathing) gcr_breathe = gcr_desired;
            }
            return false;
        case L_BRD:
            if (record->event.pressed) {
                if (LED_GCR_STEP > gcr_desired) gcr_desired = 0;
                else gcr_desired -= LED_GCR_STEP;
                if (led_animation_breathing) gcr_breathe = gcr_desired;
            }
            return false;
        case L_PTN:
            if (record->event.pressed) {
                if (led_animation_id == led_setups_count - 1) led_animation_id = 0;
                else led_animation_id++;
            }
            return false;
        case L_PTP:
            if (record->event.pressed) {
                if (led_animation_id == 0) led_animation_id = led_setups_count - 1;
                else led_animation_id--;
            }
            return false;
        case L_PSI:
            if (record->event.pressed) {
                led_animation_speed += ANIMATION_SPEED_STEP;
            }
            return false;
        case L_PSD:
            if (record->event.pressed) {
                led_animation_speed -= ANIMATION_SPEED_STEP;
                if (led_animation_speed < 0) led_animation_speed = 0;
            }
            return false;
        case L_T_MD:
            if (record->event.pressed) {
                led_lighting_mode++;
                if (led_lighting_mode > LED_MODE_MAX_INDEX) led_lighting_mode = LED_MODE_NORMAL;
            }
            return false;
        case L_T_ONF:
            if (record->event.pressed) {
                led_enabled = !led_enabled;
                I2C3733_Control_Set(led_enabled);
            }
            return false;
        case L_ON:
            if (record->event.pressed) {
                led_enabled = 1;
                I2C3733_Control_Set(led_enabled);
            }
            return false;
        case L_OFF:
            if (record->event.pressed) {
                led_enabled = 0;
                I2C3733_Control_Set(led_enabled);
            }
            return false;
        case L_T_BR:
            if (record->event.pressed) {
                led_animation_breathing = !led_animation_breathing;
                if (led_animation_breathing) {
                    gcr_breathe = gcr_desired;
                    led_animation_breathe_cur = BREATHE_MIN_STEP;
                    breathe_dir = 1;
                }
            }
            return false;
        case L_T_PTD:
            if (record->event.pressed) {
                led_animation_direction = !led_animation_direction;
            }
            return false;
        case U_T_AGCR:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_gcr_auto, "USB GCR auto mode");
            }
            return false;
        case DBG_TOG:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_enable, "Debug mode");
            }
            return false;
        case DBG_MTRX:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_matrix, "Debug matrix");
            }
            return false;
        case DBG_KBD:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_keyboard, "Debug keyboard");
            }
            return false;
        case DBG_MOU:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_mouse, "Debug mouse");
            }
            return false;
        case MD_BOOT:
            if (record->event.pressed) {
                key_timer = timer_read32();
            } else {
                if (timer_elapsed32(key_timer) >= 500) {
                    reset_keyboard();
                }
            }
            return false;
        case LOCK_SCN:
            if (record->event.pressed) {
                SEND_STRING(SS_DOWN(X_LGUI) SS_DOWN(X_LCTRL) SS_TAP(X_Q) SS_UP(X_LCTRL) SS_UP(X_LGUI));
            }
            return false;
        case KC_CAPS:
            if (record->event.pressed) {
                caps_lock_on = !caps_lock_on;
            }
            // fall through
        default:
            return true; //Process all other keycodes normally
    }
}

void maybe_do_fade_on_keypress(keyrecord_t *record) {
  if (record->event.pressed) {
    keypos_t key = record->event.key;
    uint8_t led_index = map_key_position_to_led_index(key.col, key.row);
    if (led_enabled && led_animation_id == 0 && led_index != NO_LED) {
      led_boosts[led_index] = LED_BOOST_PEAK;
      *led_map[led_index].rgb.r = led_boosts[led_index];
      *led_map[led_index].rgb.g = led_boosts[led_index];
      *led_map[led_index].rgb.b = led_boosts[led_index];
    }
  }
}

static void step_down_all_boosts(void) {
  for (int i = 0; i < ISSI3733_LED_COUNT; i++) {
    led_boosts[i] = (led_boosts[i] - LED_BOOST_STEP_DOWN < 0) ? 0 : led_boosts[i] - LED_BOOST_STEP_DOWN;
  }
}

static void update_all_leds(void) {
  for (int i = 0; i < ISSI3733_LED_COUNT; i++) {
    if (i == USB_LED_CAPS_LOCK_SCANCODE && caps_lock_on) {
      *led_map[i].rgb.r = LED_BOOST_PEAK;
      *led_map[i].rgb.g = LED_BOOST_PEAK;
      *led_map[i].rgb.b = LED_BOOST_PEAK;
    } else {
      *led_map[i].rgb.r = led_boosts[i];
      *led_map[i].rgb.g = led_boosts[i];
      *led_map[i].rgb.b = led_boosts[i];
    }
  }
}

static uint8_t map_key_position_to_led_index(uint8_t col, uint8_t row) {
  if (row >= 0 && row < MATRIX_ROWS && col >= 0 && col < MATRIX_COLS) {
    return g_led_config.matrix_co[row][col];
  }
  return NO_LED;
}

led_instruction_t led_instructions[] = {
    //Please see ../default_md/keymap.c for examples

    //All LEDs use the user's selected pattern (this is the factory default)
    { .flags = LED_FLAG_USE_ROTATE_PATTERN },

    //end must be set to 1 to indicate end of instruction set
     { .end = 1 }
};
