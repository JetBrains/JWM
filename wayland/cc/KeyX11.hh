#pragma once

#include <cstdint>
#include "Key.hh"

namespace jwm {
    namespace KeyX11 {
        jwm::Key fromNative(uint32_t v);
        bool getKeyState(jwm::Key key);
        void setKeyState(jwm::Key key, bool isDown);
        int getModifiers();
    }
}