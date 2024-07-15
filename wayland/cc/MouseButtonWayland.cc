#include "MouseButtonWayland.hh"
#include <linux/input.h>

jwm::MouseButton jwm::MouseButtonWayland::fromNative(uint32_t v) {
    switch (v) {
        case BTN_LEFT: return jwm::MouseButton::PRIMARY; 
        case BTN_MIDDLE: return jwm::MouseButton::MIDDLE;
        case BTN_RIGHT: return jwm::MouseButton::SECONDARY;
        // TODO: is this mapping consistent?
        // I've gotten this from observing my mouse
        case BTN_SIDE:
        case BTN_BACK: return jwm::MouseButton::BACK;
        case BTN_EXTRA:
        case BTN_FORWARD: return jwm::MouseButton::FORWARD;
    }
    return jwm::MouseButton::PRIMARY;
}

bool jwm::MouseButtonWayland::isButton(uint32_t v) {
    return v >= 0x110 && v <= 0x116; // mouse wheel buttons
}

int jwm::MouseButtonWayland::fromNativeMask(unsigned v) {
    int res = 0;
    if (v & 0x100) res |= int(jwm::MouseButton::PRIMARY);
    if (v & 0x400) res |= int(jwm::MouseButton::SECONDARY);
    if (v & 0x200) res |= int(jwm::MouseButton::MIDDLE);
    return res;
}
