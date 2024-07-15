#include "ILayerWayland.hh"
#include <wayland-client.h>
#include "WindowWayland.hh"

void jwm::ILayerWayland::detachBuffer() {
    if (fWindow && fWindow->_waylandWindow) {
        wl_surface_set_buffer_scale(fWindow->_waylandWindow, 1);
    }
}
