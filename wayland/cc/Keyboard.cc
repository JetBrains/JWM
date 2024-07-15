#include "Keyboard.hh"
#include "WindowManagerWayland.hh"
#include "WindowWayland.hh"
#include "KeyWayland.hh"
#include <unistd.h>
#include <impl/JNILocal.hh>
#include "StringUTF16.hh"
#include <system_error>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include "AppWayland.hh"
#include <chrono>
#include <clocale>
#include <xkbcommon/xkbcommon-names.h>
#include <algorithm>

using namespace jwm;

// I've noticed that pointers to lambdas are null for some reason. 
// No idea what's wrong. Going to cry myself to sleep tonight.
static void kbKeymap(void* data, wl_keyboard* kb, uint32_t format, int32_t fd, uint32_t size) {
    auto self = reinterpret_cast<Keyboard*>(data);
    
    
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        fprintf(stderr, "no xkb keymap\n");
        return;
    }

    char* map_str = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (map_str == MAP_FAILED) {
        close(fd);
        fprintf(stderr, "keymap mmap failed: %s", strerror(errno));
        return;
    }

    xkb_keymap* keymap = xkb_keymap_new_from_string(
                self->_context, map_str,
                XKB_KEYMAP_FORMAT_TEXT_V1,
                XKB_KEYMAP_COMPILE_NO_FLAGS
            );
    munmap(map_str, size);
    close(fd);

    if (!keymap) {
        return;
    }
    self->_state = xkb_state_new(keymap);

    self->_keymap = keymap;
   
    const char* locale = std::setlocale(LC_CTYPE, nullptr);

    self->_composeTable = xkb_compose_table_new_from_locale(self->_context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    self->_composeState = xkb_compose_state_new(self->_composeTable, XKB_COMPOSE_STATE_NO_FLAGS);

}
static void kbEnter(void* data, wl_keyboard* kb, uint32_t serial, wl_surface* surface,
        wl_array *keys) {
    auto self = reinterpret_cast<Keyboard*>(data);
    auto win = self->_wm.getWindowForNative(surface);
    if (!win) return;
    self->_serial = serial;
    self->_focus = jwm::ref(win);
    if (self->_state) {
        uint32_t* key;
        // C++ jank
        // Normal macro fails to compile bc `void*` can't implicitly convert into `uint32_t*`
        for (key = (uint32_t*)keys->data;
                (const char*) key < (const char*)keys->data + keys->size;
                key++
            ) {
            auto jwmKey = jwm::KeyWayland::fromNative(*key + 8);
            self->submitKey(jwmKey, WL_KEYBOARD_KEY_STATE_PRESSED);
        }
    }
}
static void kbLeave(void* data, wl_keyboard* kb, uint32_t serial, wl_surface* surface) {
    auto self = reinterpret_cast<Keyboard*>(data);
    std::list<jwm::Key> liftedKeys(self->_depressedKeys);
    for (auto key : liftedKeys) {
        self->submitKey(key, WL_KEYBOARD_KEY_STATE_RELEASED);
    }
    self->_repeating = false;
    self->_repeatingText = false;
    self->_serial = 0;
    if (self->_focus)
        jwm::unref(&self->_focus);
}
static void kbKey(void* data, wl_keyboard* kb, uint32_t serial, uint32_t time,
                uint32_t key, uint32_t state) {
    auto self = reinterpret_cast<Keyboard*>(data);
    if (!self->_state || !self->_focus) return;

    const xkb_keysym_t *syms;
    uint32_t keyCode = key + 8;
    if (xkb_state_key_get_syms(self->_state, keyCode, &syms) != 1) {
        xkb_compose_state_feed(self->_composeState, XKB_KEY_NoSymbol);
        return;
    }
    auto sym = syms[0];
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
        xkb_compose_state_feed(self->_composeState, sym);
    auto status = xkb_compose_state_get_status(self->_composeState);
    bool composeRelated = status != XKB_COMPOSE_NOTHING;
    // use raw key code
    jwm::Key jwmKey = KeyWayland::fromNative(keyCode);
    self->_repeatingText = false;
    self->_repeating = false;
    self->submitKey(jwmKey, state);
    if (composeRelated) {
        int dacount;
        switch (status) {
            case XKB_COMPOSE_COMPOSING:
                break;
            case XKB_COMPOSE_COMPOSED:
                // I am going to wager a guess that no one will ever have a compose key that binds to a 
                // key we actually parse. 
                // auto keysym = xkb_compose_state_get_one_sym(self->_composeState);
                char textBuf[0x40];

                dacount = xkb_compose_state_get_utf8(self->_composeState, textBuf, sizeof(textBuf) - 1);

                if (dacount > 0 && (dacount < sizeof(textBuf) - 1)) {
                    JNIEnv* env = jwm::app.getJniEnv();

                    jwm::StringUTF16 converted = reinterpret_cast<const char*>(textBuf);
                    jwm::JNILocal<jstring> jtext = converted.toJString(env);

                    jwm::JNILocal<jobject> eventTextInput(env, classes::EventTextInput::make(env, jtext.get()));
                    

                    self->_focus->dispatch(eventTextInput.get());

                }


                xkb_compose_state_reset(self->_composeState);
                break;
            case XKB_COMPOSE_CANCELLED:
                xkb_compose_state_reset(self->_composeState);
                break;
        }
        return;
    }   
    if (state != WL_KEYBOARD_KEY_STATE_PRESSED) {
        return;
    }
    // ??? 
    self->_lastPress = std::chrono::steady_clock::now();
    self->_repeatKey = jwmKey;
    bool shouldRepeat = xkb_keymap_key_repeats(self->_keymap, keyCode) && (self->_repeatRate > 0);
    if (shouldRepeat && (jwmKey != jwm::Key::UNDEFINED)) {
        self->_repeating = true;
        self->_nextRepeat = self->_lastPress + std::chrono::milliseconds(self->_repeatDelay);
        self->_wm.notifyLoop();
    }
    char textBuffer[0x40];
    int count = xkb_state_key_get_utf8(self->_state, keyCode, textBuffer, sizeof(textBuffer)-1);
    // ???
    if (count >= sizeof(textBuffer) - 1) {
        return;
    }
    if (count > 0) {
        // ignore sinful control symbols
        if (textBuffer[0] != 127 && textBuffer[0] > 0x1f) {
            JNIEnv* env = jwm::app.getJniEnv();

            jwm::StringUTF16 converted = reinterpret_cast<const char*>(textBuffer);
            self->_repeatText = converted;
            if (shouldRepeat)
                self->_repeatingText = true;
            jwm::JNILocal<jstring> jtext = converted.toJString(env);

            jwm::JNILocal<jobject> eventTextInput(env, classes::EventTextInput::make(env, jtext.get()));
            

            self->_focus->dispatch(eventTextInput.get());
        }
    }
} 
void kbModifiers(void* data, wl_keyboard* kb, uint32_t serial, uint32_t mods_depressed, 
            uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    auto self = reinterpret_cast<Keyboard*>(data);
    if (!self->_state) return;
    xkb_state_update_mask(self->_state,
            mods_depressed, mods_latched, mods_locked,
            0, 0, group);
}
void kbRepeatInfo(void* data, wl_keyboard* kb, int32_t rate, int32_t delay) {
    auto self = reinterpret_cast<Keyboard*>(data);
    self->_repeatRate = rate;
    self->_repeatDelay = delay;
}

wl_keyboard_listener Keyboard::_keyboardListener = {
    .keymap = kbKeymap,
    .enter = kbEnter,
    .leave = kbLeave,
    .key = kbKey,
    .modifiers = kbModifiers,
    .repeat_info = kbRepeatInfo
};
Keyboard::Keyboard(wl_keyboard* kb, WindowManagerWayland* wm):
    _keyboard(kb),
    _wm(*wm)
{
    wl_keyboard_add_listener(kb, &_keyboardListener, this);
    _context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
}

Keyboard::~Keyboard()
{
    if (_keyboard)
        wl_keyboard_release(_keyboard);
    if (_context)
        xkb_context_unref(_context);
    if (_keymap)
        xkb_keymap_unref(_keymap);
}

void Keyboard::submitKey(jwm::Key key, uint32_t state) {
    if (key != jwm::Key::UNDEFINED) {
        jwm::KeyLocation location = jwm::KeyLocation::DEFAULT;
        JNILocal<jobject> keyEvent(
            jwm::app.getJniEnv(),
            classes::EventKey::make(
                    jwm::app.getJniEnv(),
                    key,
                    state == WL_KEYBOARD_KEY_STATE_PRESSED,
                    KeyWayland::getModifiers(_state),
                    location
                )
            );
        _focus->dispatch(keyEvent.get());

        if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            _depressedKeys.push_back(key);
        } else {
            auto it = std::find(_depressedKeys.begin(), _depressedKeys.end(), key);
            if (it != _depressedKeys.end()) {
                _depressedKeys.erase(it);
            }
        }
    }
}

