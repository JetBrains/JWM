#include "KeyWayland.hh"
#include "KeyModifier.hh"
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-names.h>
#include <linux/input.h>

int jwm::KeyWayland::getModifiers(xkb_state* state) {

    int m = 0;
    
    if (!state)
        return 0;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE)) m |= (int)jwm::KeyModifier::SHIFT;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL , XKB_STATE_MODS_EFFECTIVE)) m |= (int)jwm::KeyModifier::CONTROL;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT  , XKB_STATE_MODS_EFFECTIVE)) m |= (int)jwm::KeyModifier::ALT;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_LOGO , XKB_STATE_MODS_EFFECTIVE)) m |= (int)jwm::KeyModifier::LINUX_META;
    if (xkb_state_mod_name_is_active(state, "Super"           , XKB_STATE_MODS_EFFECTIVE)) m |= (int)jwm::KeyModifier::LINUX_SUPER;

    return m;
}
/*
int jwm::KeyWayland::getModifiersFromMask(int mask) {
    int m = getModifiers();
    // ???
    // if (mask & ShiftMask  ) m |= (int)jwm::KeyModifier::SHIFT;
    // if (mask & ControlMask) m |= (int)jwm::KeyModifier::CONTROL;
    // if (mask & Mod1Mask   ) m |= (int)jwm::KeyModifier::ALT;

    return m;
}*/

jwm::Key jwm::KeyWayland::fromNative(uint32_t v) { 
    switch (v - 8) {
        // Modifiers
        case KEY_CAPSLOCK: return Key::CAPS_LOCK;
        case KEY_RIGHTSHIFT:
        case KEY_LEFTSHIFT: return Key::SHIFT;
        case KEY_RIGHTCTRL:
        case KEY_LEFTCTRL: return Key::CONTROL;
        case KEY_RIGHTALT:
        case KEY_LEFTALT: return Key::ALT;
        // Key::WIN_LOGO
        case KEY_LEFTMETA:
        case KEY_RIGHTMETA: return Key::LINUX_SUPER;
        // prefer super over meta
        // KEY::LINUX_META
        // Key::MAC_COMMAND
        // Key::MAC_OPTION
        // Key::MAC_FN

        // Rest of the keys
        case KEY_ENTER: return Key::ENTER;
        case KEY_BACKSPACE: return Key::BACKSPACE;
        case KEY_TAB: return Key::TAB;
        case KEY_CANCEL: return Key::CANCEL;
        case KEY_CLEAR: return Key::CLEAR;
        case KEY_PAUSE: return Key::PAUSE;
        case KEY_ESC: return Key::ESCAPE;
        case KEY_SPACE: return Key::SPACE;
        case KEY_PAGEUP: return Key::PAGE_UP;
        case KEY_PAGEDOWN: return Key::PAGE_DOWN;
        case KEY_END: return Key::END;
        case KEY_HOME: return Key::HOME;
        case KEY_LEFT: return Key::LEFT;
        case KEY_UP: return Key::UP;
        case KEY_RIGHT: return Key::RIGHT;
        case KEY_DOWN: return Key::DOWN;
        case KEY_COMMA: return Key::COMMA;
        case KEY_MINUS: return Key::MINUS;
        case KEY_DOT: return Key::PERIOD;
        case KEY_SLASH: return Key::SLASH;
        case KEY_0: return Key::DIGIT0;
        case KEY_1: return Key::DIGIT1;
        case KEY_2: return Key::DIGIT2;
        case KEY_3: return Key::DIGIT3;
        case KEY_4: return Key::DIGIT4;
        case KEY_5: return Key::DIGIT5;
        case KEY_6: return Key::DIGIT6;
        case KEY_7: return Key::DIGIT7;
        case KEY_8: return Key::DIGIT8;
        case KEY_9: return Key::DIGIT9;
        case KEY_SEMICOLON: return Key::SEMICOLON;
        case KEY_EQUAL: return Key::EQUALS;
        case KEY_A: return Key::A;
        case KEY_B: return Key::B;
        case KEY_C: return Key::C;
        case KEY_D: return Key::D;
        case KEY_E: return Key::E;
        case KEY_F: return Key::F;
        case KEY_G: return Key::G;
        case KEY_H: return Key::H;
        case KEY_I: return Key::I;
        case KEY_J: return Key::J;
        case KEY_K: return Key::K;
        case KEY_L: return Key::L;
        case KEY_M: return Key::M;
        case KEY_N: return Key::N;
        case KEY_O: return Key::O;
        case KEY_P: return Key::P;
        case KEY_Q: return Key::Q;
        case KEY_R: return Key::R;
        case KEY_S: return Key::S;
        case KEY_T: return Key::T;
        case KEY_U: return Key::U;
        case KEY_V: return Key::V;
        case KEY_W: return Key::W;
        case KEY_X: return Key::X;
        case KEY_Y: return Key::Y;
        case KEY_Z: return Key::Z;
        case KEY_LEFTBRACE: return Key::OPEN_BRACKET;
        case KEY_BACKSLASH: return Key::BACK_SLASH;
        case KEY_RIGHTBRACE: return Key::CLOSE_BRACKET;
        case KEY_KP0: return Key::DIGIT0;
        case KEY_KP1: return Key::DIGIT1;
        case KEY_KP2: return Key::DIGIT2;
        case KEY_KP3: return Key::DIGIT3;
        case KEY_KP4: return Key::DIGIT4;
        case KEY_KP5: return Key::DIGIT5;
        case KEY_KP6: return Key::DIGIT6;
        case KEY_KP7: return Key::DIGIT7;
        case KEY_KP8: return Key::DIGIT8;
        case KEY_KP9: return Key::DIGIT9;
        case KEY_KPASTERISK: return Key::MULTIPLY;
        case KEY_KPPLUS: return Key::ADD;
        case KEY_KPCOMMA: return Key::SEPARATOR;
        case KEY_KPMINUS: return Key::MINUS;
        case KEY_KPDOT: return Key::PERIOD;
        case KEY_KPSLASH: return Key::SLASH;
        // no kp delete?
        // case KEY_: return Key::DEL;
        case KEY_DELETE: return Key::DEL;
        case KEY_NUMLOCK: return Key::NUM_LOCK;
        case KEY_SCROLLLOCK: return Key::SCROLL_LOCK;
        case KEY_F1: return Key::F1;
        case KEY_F2: return Key::F2;
        case KEY_F3: return Key::F3;
        case KEY_F4: return Key::F4;
        case KEY_F5: return Key::F5;
        case KEY_F6: return Key::F6;
        case KEY_F7: return Key::F7;
        case KEY_F8: return Key::F8;
        case KEY_F9: return Key::F9;
        case KEY_F10: return Key::F10;
        case KEY_F11: return Key::F11;
        case KEY_F12: return Key::F12;
        case KEY_F13: return Key::F13;
        case KEY_F14: return Key::F14;
        case KEY_F15: return Key::F15;
        case KEY_F16: return Key::F16;
        case KEY_F17: return Key::F17;
        case KEY_F18: return Key::F18;
        case KEY_F19: return Key::F19;
        case KEY_F20: return Key::F20;
        case KEY_F21: return Key::F21;
        case KEY_F22: return Key::F22;
        case KEY_F23: return Key::F23;
        case KEY_F24: return Key::F24;
        case KEY_PRINT: return Key::PRINTSCREEN;
        case KEY_INSERT: return Key::INSERT;
        case KEY_HELP: return Key::HELP;
        case KEY_GRAVE: return Key::BACK_QUOTE;
        case KEY_APOSTROPHE: return Key::QUOTE;
        case KEY_MENU: return Key::MENU;
        // Key::KANA
        case KEY_VOLUMEUP: return Key::VOLUME_UP;
        case KEY_VOLUMEDOWN: return Key::VOLUME_DOWN;
        case KEY_MUTE: return Key::MUTE;
        default: return Key::UNDEFINED;
    } 
}
