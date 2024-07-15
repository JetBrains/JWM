#pragma once

#include <cstdint>
#include "Key.hh"
#include "KeyLocation.hh"

namespace jwm {
    namespace KeyX11 {
        jwm::Key fromNative(uint32_t v, jwm::KeyLocation& location, int& modifiers);
        bool getKeyState(jwm::Key key);
        void setKeyState(jwm::Key key, bool isDown);
        int getModifiers();
        int getModifiersFromMask(int mask);
    }
}