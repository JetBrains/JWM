#pragma once

#include <cstdint>
#include "MouseButton.hh"

namespace jwm {
    namespace MouseButtonX11 {
        MouseButton fromNative(uint32_t v);
        int fromNativeMask(unsigned v);
        bool isButton(uint32_t v);
    }
}