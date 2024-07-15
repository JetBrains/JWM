#pragma once

#include <cstdint>
#include "Key.hh"
#include <xkbcommon/xkbcommon.h>

namespace jwm {
    namespace KeyWayland {
        jwm::Key fromNative(uint32_t v);
        int getModifiers(xkb_state* state);
        // int getModifiersFromMask(int mask);
    }
}
