#include <wayland-xdg-activation-v1-client-protocol.h>
#include <string>
#include <wayland-client.h>

namespace jwm {
    class WindowManagerWayland;
    struct Token {
        std::string token;
        // blocks
        static Token make(WindowManagerWayland& wm, wl_surface* surface);

        void grab(WindowManagerWayland& wm, wl_surface* surface);
    }; 
}
