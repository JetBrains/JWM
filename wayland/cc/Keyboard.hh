#pragma once

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <chrono>
#include "KeyWayland.hh"
#include "StringUTF16.hh"
#include <list>

namespace jwm {
    class WindowManagerWayland;
    class WindowWayland;
    class Keyboard {
    public:
        Keyboard(wl_keyboard* kb, jwm::WindowManagerWayland* wm);
        ~Keyboard();

        wl_keyboard* _keyboard;
        wl_keyboard* getKeyboard() const {
            return _keyboard;
        }
        xkb_context* _context = nullptr;
        xkb_state* _state = nullptr;
        xkb_keymap* _keymap = nullptr;
        xkb_compose_table* _composeTable = nullptr;
        xkb_compose_state* _composeState = nullptr;

        xkb_state* getState() const {
            return _state;
        }
        jwm::WindowWayland* _focus = nullptr;
        jwm::WindowWayland* getFocus() const {
            return _focus;
        }
        uint32_t _serial = 0;
        uint32_t getSerial() const {
            return _serial;
        }
        std::chrono::time_point<std::chrono::steady_clock> _lastPress;
        std::chrono::time_point<std::chrono::steady_clock> _nextRepeat;
        int32_t _repeatRate = 100;
        int32_t _repeatDelay = 300;

        void submitKey(jwm::Key key, uint32_t state);

        jwm::StringUTF16 _repeatText;
        jwm::Key _repeatKey = jwm::Key::UNDEFINED;
        bool _repeating = false;
        bool _repeatingText = false;

        std::list<jwm::Key> _depressedKeys;

        jwm::WindowManagerWayland& _wm;

        static wl_keyboard_listener _keyboardListener;
    private:
        // no copy or move
        Keyboard(const Keyboard&) = delete;
        Keyboard(Keyboard&&) = delete;
        Keyboard& operator=(const Keyboard&) = delete;
        Keyboard& operator=(Keyboard&&) = delete;
    };
}
