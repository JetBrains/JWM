#pragma once

#include <cstdint>
#include "MouseButton.hh"

namespace jwm {
    namespace MouseButtonWayland {
        MouseButton fromNative(uint32_t v);
        int fromNativeMask(unsigned v);
        bool isButton(uint32_t v);
    }
}
