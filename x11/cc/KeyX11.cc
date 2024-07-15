#include "KeyX11.hh"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include "KeyModifier.hh"


bool gKeyStates[(size_t) jwm::Key::_KEY_COUNT] = {0};

bool jwm::KeyX11::getKeyState(jwm::Key key) {
    return gKeyStates[(size_t) key];
}

void jwm::KeyX11::setKeyState(jwm::Key key, bool isDown) {
    gKeyStates[(size_t) key] = isDown;
}

int jwm::KeyX11::getModifiers() {
    int m = 0;

    if (getKeyState(jwm::Key::SHIFT      )) m |= (int)jwm::KeyModifier::SHIFT;
    if (getKeyState(jwm::Key::CONTROL    )) m |= (int)jwm::KeyModifier::CONTROL;
    if (getKeyState(jwm::Key::ALT        )) m |= (int)jwm::KeyModifier::ALT;
    if (getKeyState(jwm::Key::LINUX_META )) m |= (int)jwm::KeyModifier::LINUX_META;
    if (getKeyState(jwm::Key::LINUX_SUPER)) m |= (int)jwm::KeyModifier::LINUX_SUPER;

    return m;
}

int jwm::KeyX11::getModifiersFromMask(int mask) {
    int m = getModifiers();

    if (mask & ShiftMask  ) m |= (int)jwm::KeyModifier::SHIFT;
    if (mask & ControlMask) m |= (int)jwm::KeyModifier::CONTROL;
    if (mask & Mod1Mask   ) m |= (int)jwm::KeyModifier::ALT;

    return m;
}

jwm::Key jwm::KeyX11::fromNative(uint32_t v, jwm::KeyLocation& location, int& modifiers) {
    location = jwm::KeyLocation::DEFAULT;
    modifiers = 0;
    switch (v) {
        // Modifiers
        case XK_Caps_Lock: return Key::CAPS_LOCK;
        case XK_Shift_R: location = jwm::KeyLocation::RIGHT; // fallthrough
        case XK_Shift_L: return Key::SHIFT;
        case XK_Control_R: location = jwm::KeyLocation::RIGHT; // fallthrough
        case XK_Control_L: return Key::CONTROL;
        case XK_Alt_R: location = jwm::KeyLocation::RIGHT; // fallthrough
        case XK_Alt_L: return Key::ALT;
        // Key::WIN_LOGO
        case XK_Super_R: location = jwm::KeyLocation::RIGHT; // fallthrough
        case XK_Super_L: return Key::LINUX_SUPER;
        case XK_Meta_R: location = jwm::KeyLocation::RIGHT; // fallthrough
        case XK_Meta_L: return Key::LINUX_META;
        // Key::MAC_COMMAND
        // Key::MAC_OPTION
        // Key::MAC_FN

        // Rest of the keys
        case XK_Return: return Key::ENTER;
        case XK_BackSpace: return Key::BACKSPACE;
        case XK_Tab: return Key::TAB;
        case XK_Cancel: return Key::CANCEL;
        case XK_Clear: return Key::CLEAR;
        case XK_Pause: return Key::PAUSE;
        case XK_Escape: return Key::ESCAPE;
        case XK_space: return Key::SPACE;
        case XK_Page_Up: return Key::PAGE_UP;
        case XK_Page_Down: return Key::PAGE_DOWN;
        case XK_End: return Key::END;
        case XK_Home: return Key::HOME;
        case XK_Left: return Key::LEFT;
        case XK_Up: return Key::UP;
        case XK_Right: return Key::RIGHT;
        case XK_Down: return Key::DOWN;
        case XK_comma: return Key::COMMA;
        case XK_minus: return Key::MINUS;
        case XK_period: return Key::PERIOD;
        case XK_slash: return Key::SLASH;
        case XK_0: return Key::DIGIT0;
        case XK_1: return Key::DIGIT1;
        case XK_2: return Key::DIGIT2;
        case XK_3: return Key::DIGIT3;
        case XK_4: return Key::DIGIT4;
        case XK_5: return Key::DIGIT5;
        case XK_6: return Key::DIGIT6;
        case XK_7: return Key::DIGIT7;
        case XK_8: return Key::DIGIT8;
        case XK_9: return Key::DIGIT9;
        case XK_semicolon: return Key::SEMICOLON;
        case XK_equal: return Key::EQUALS;
        case XK_a: return Key::A;
        case XK_b: return Key::B;
        case XK_c: return Key::C;
        case XK_d: return Key::D;
        case XK_e: return Key::E;
        case XK_f: return Key::F;
        case XK_g: return Key::G;
        case XK_h: return Key::H;
        case XK_i: return Key::I;
        case XK_j: return Key::J;
        case XK_k: return Key::K;
        case XK_l: return Key::L;
        case XK_m: return Key::M;
        case XK_n: return Key::N;
        case XK_o: return Key::O;
        case XK_p: return Key::P;
        case XK_q: return Key::Q;
        case XK_r: return Key::R;
        case XK_s: return Key::S;
        case XK_t: return Key::T;
        case XK_u: return Key::U;
        case XK_v: return Key::V;
        case XK_w: return Key::W;
        case XK_x: return Key::X;
        case XK_y: return Key::Y;
        case XK_z: return Key::Z;
        case XK_bracketleft: return Key::OPEN_BRACKET;
        case XK_backslash: return Key::BACK_SLASH;
        case XK_bracketright: return Key::CLOSE_BRACKET;
        case XK_KP_0: case XK_KP_Insert:    location = jwm::KeyLocation::KEYPAD; return Key::DIGIT0;
        case XK_KP_1: case XK_KP_End:       location = jwm::KeyLocation::KEYPAD; return Key::DIGIT1;
        case XK_KP_2: case XK_KP_Down:      location = jwm::KeyLocation::KEYPAD; return Key::DIGIT2;
        case XK_KP_3: case XK_KP_Page_Down: location = jwm::KeyLocation::KEYPAD; return Key::DIGIT3;
        case XK_KP_4: case XK_KP_Left:      location = jwm::KeyLocation::KEYPAD; return Key::DIGIT4;
        case XK_KP_5: case XK_KP_Begin:     location = jwm::KeyLocation::KEYPAD; return Key::DIGIT5;
        case XK_KP_6: case XK_KP_Right:     location = jwm::KeyLocation::KEYPAD; return Key::DIGIT6;
        case XK_KP_7: case XK_KP_Home:      location = jwm::KeyLocation::KEYPAD; return Key::DIGIT7;
        case XK_KP_8: case XK_KP_Up:        location = jwm::KeyLocation::KEYPAD; return Key::DIGIT8;
        case XK_KP_9: case XK_KP_Page_Up:   location = jwm::KeyLocation::KEYPAD; return Key::DIGIT9;
        case XK_KP_Add:       location = jwm::KeyLocation::KEYPAD; return Key::ADD;
        case XK_KP_Separator: location = jwm::KeyLocation::KEYPAD; return Key::SEPARATOR;
        case XK_KP_Subtract:  location = jwm::KeyLocation::KEYPAD; return Key::MINUS;
        case XK_KP_Decimal:   location = jwm::KeyLocation::KEYPAD; return Key::PERIOD;
        case XK_KP_Divide:    location = jwm::KeyLocation::KEYPAD; return Key::SLASH;
        case XK_KP_Delete:    location = jwm::KeyLocation::KEYPAD; return Key::DEL;
        case XK_KP_Enter:     location = jwm::KeyLocation::KEYPAD; return Key::ENTER;
        case XK_KP_Multiply:  location = jwm::KeyLocation::KEYPAD; return Key::MULTIPLY;
        case XK_multiply: return Key::MULTIPLY;
        case XK_Delete: return Key::DEL;
        case XK_Num_Lock: return Key::NUM_LOCK;
        case XK_Scroll_Lock: return Key::SCROLL_LOCK;
        case XK_F1: return Key::F1;
        case XK_F2: return Key::F2;
        case XK_F3: return Key::F3;
        case XK_F4: return Key::F4;
        case XK_F5: return Key::F5;
        case XK_F6: return Key::F6;
        case XK_F7: return Key::F7;
        case XK_F8: return Key::F8;
        case XK_F9: return Key::F9;
        case XK_F10: return Key::F10;
        case XK_F11: return Key::F11;
        case XK_F12: return Key::F12;
        case XK_F13: return Key::F13;
        case XK_F14: return Key::F14;
        case XK_F15: return Key::F15;
        case XK_F16: return Key::F16;
        case XK_F17: return Key::F17;
        case XK_F18: return Key::F18;
        case XK_F19: return Key::F19;
        case XK_F20: return Key::F20;
        case XK_F21: return Key::F21;
        case XK_F22: return Key::F22;
        case XK_F23: return Key::F23;
        case XK_F24: return Key::F24;
        case XK_Print: return Key::PRINTSCREEN;
        case XK_Insert: return Key::INSERT;
        case XK_Help: return Key::HELP;
        case XK_grave: return Key::BACK_QUOTE;
        case XK_quoteright: return Key::QUOTE;
        case XK_Menu: return Key::MENU;
        // Key::KANA
        // Key::VOLUME_UP
        // Key::VOLUME_DOWN
        // Key::MUTE
        case XK_exclam:      modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT1;
        case XK_quotedbl:    modifiers = (int)jwm::KeyModifier::SHIFT; return Key::QUOTE;
        case XK_numbersign:  modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT3;
        case XK_dollar:      modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT4;
        case XK_percent:     modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT5;
        case XK_ampersand:   modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT7;
        case XK_parenleft:   modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT9;
        case XK_parenright:  modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT0;
        case XK_asterisk:    modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT8;
        case XK_plus:        return Key::ADD;
        case XK_colon:       modifiers = (int)jwm::KeyModifier::SHIFT; return Key::SEMICOLON;
        case XK_less:        modifiers = (int)jwm::KeyModifier::SHIFT; return Key::COMMA;
        case XK_greater:     modifiers = (int)jwm::KeyModifier::SHIFT; return Key::PERIOD;
        case XK_question:    modifiers = (int)jwm::KeyModifier::SHIFT; return Key::SLASH;
        case XK_at:          modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT2;
        case XK_asciicircum: modifiers = (int)jwm::KeyModifier::SHIFT; return Key::DIGIT6;
        case XK_underscore:  modifiers = (int)jwm::KeyModifier::SHIFT; return Key::MINUS;
        case XK_braceleft:   modifiers = (int)jwm::KeyModifier::SHIFT; return Key::OPEN_BRACKET;
        case XK_bar:         modifiers = (int)jwm::KeyModifier::SHIFT; return Key::BACK_SLASH;
        case XK_braceright:  modifiers = (int)jwm::KeyModifier::SHIFT; return Key::CLOSE_BRACKET;
        case XK_asciitilde:  modifiers = (int)jwm::KeyModifier::SHIFT; return Key::BACK_QUOTE;
        case XK_A: return Key::A;
        case XK_B: return Key::B;
        case XK_C: return Key::C;
        case XK_D: return Key::D;
        case XK_E: return Key::E;
        case XK_F: return Key::F;
        case XK_G: return Key::G;
        case XK_H: return Key::H;
        case XK_I: return Key::I;
        case XK_J: return Key::J;
        case XK_K: return Key::K;
        case XK_L: return Key::L;
        case XK_M: return Key::M;
        case XK_N: return Key::N;
        case XK_O: return Key::O;
        case XK_P: return Key::P;
        case XK_Q: return Key::Q;
        case XK_R: return Key::R;
        case XK_S: return Key::S;
        case XK_T: return Key::T;
        case XK_U: return Key::U;
        case XK_V: return Key::V;
        case XK_W: return Key::W;
        case XK_X: return Key::X;
        case XK_Y: return Key::Y;
        case XK_Z: return Key::Z;
        default: return Key::UNDEFINED;
    }
}
