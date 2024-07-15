#include "Decoration.hh"
#include "WindowWayland.hh"
#include "WindowManagerWayland.hh"
#include <cstring>

using namespace jwm;

static unsigned int grey_data[] = {0xFF333333};
static unsigned int zero_data[] = {0x00000000};
// no image editor
// pure unadulterated programming
static unsigned int close_data[] = {
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,
    0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000,
    0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
};
static unsigned int min_data[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};
static unsigned int max_data[] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};

static void _decorationConfigure(void* data, zxdg_toplevel_decoration_v1* decoration, uint32_t mode) {
    auto self = reinterpret_cast<Decoration*>(data);
    if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
        self->_serverSide = true;
    } else {
        self->_serverSide = false;
    }
}
static zxdg_toplevel_decoration_v1_listener _decorationListener = {
    .configure = _decorationConfigure
};

static void _xdgSurfaceConfigure(void* data, xdg_surface* surface, uint32_t serial) {
    auto self = reinterpret_cast<Decoration*>(data);
    auto& window = self->_window;
    int width = 0, height = 0;
    int pendingWidth = self->_pendingWidth;
    int pendingHeight = self->_pendingHeight;
    // do it here bc we don't know configure order
    if (!self->_serverSide) {
        pendingHeight -= self->getTopSize();
    }
    if (pendingWidth > 0)
        width = pendingWidth;
    else
        width = window._floatingWidth;

    if (pendingHeight > 0)
        height = pendingHeight;
    else
        height = window._floatingHeight;
    if (self->_floating)
        self->constrainSize(width, height);
    self->_pendingWidth = 0;
    self->_pendingHeight = 0;
    if (self->_oldActive != self->_active) {
        if (self->_active)
            window.dispatch(classes::EventWindowFocusIn::kInstance);
        else
            window.dispatch(classes::EventWindowFocusOut::kInstance);
    }
    self->_oldActive = self->_active;
    if (self->_oldMaximized != self->_maximized) {
        if (self->_maximized)
            window.dispatch(classes::EventWindowMaximize::kInstance);
    }
    self->_oldMaximized = self->_maximized;
    if (self->_oldFullscreen != self->_fullscreen) {
        if (self->_fullscreen)
            window.dispatch(classes::EventWindowFullScreenEnter::kInstance);
        else
            window.dispatch(classes::EventWindowFullScreenExit::kInstance);
    }
    self->_oldFullscreen = self->_fullscreen;
    if (!self->_configured) {
        if (window._layer)
            window._layer->attachBuffer();
    }
    if (self->_serverSide) {
        if (self->_border.surface)
            self->_destroyDecorations();
    } else {
        if (!self->_border.surface)
            self->_showDecorations(!self->_isVisible);
    }
    // resize on first configure
    self->_configured = true;
    // ask to configure _before_ commit. jank.
    if (self->_floating) {
        xdg_surface_ack_configure(self->_xdgSurface, serial);
    }
    if (window.getUnscaledWidth() != width || window.getUnscaledHeight() != height) {
        if (self->_floating) {
            if (width > 0) {
                window._floatingWidth = width;
            }
            if (height > 0) {
                window._floatingHeight = height;
            }
        }
        window._adaptSize(width, height);
        self->_adaptSize();
    }

    wl_surface_commit(window._waylandWindow);
    if (!self->_floating) {

        xdg_surface_ack_configure(self->_xdgSurface, serial);
    }
}

static xdg_surface_listener _xdgSurfaceListener = {
    .configure = _xdgSurfaceConfigure
};

static void _xdgToplevelConfigure(void* data, xdg_toplevel* toplevel, int width, int height, wl_array* states) {
    auto self = reinterpret_cast<Decoration*>(data);

    self->_pendingWidth = width;
    self->_pendingHeight = height;

    bool active = false;
    bool maximized = false;
    bool fullscreen = false;
    bool floating = true;
    for (uint32_t* pos = (uint32_t*)states->data;
            (const char*)pos < ((const char*) states->data + states->size);
            pos++) {
        switch (*pos) {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                maximized = true;
                floating = false;
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                active = true;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                fullscreen = true;
                floating = false;
                break;
            case XDG_TOPLEVEL_STATE_TILED_LEFT:
            case XDG_TOPLEVEL_STATE_TILED_RIGHT:
            case XDG_TOPLEVEL_STATE_TILED_TOP:
            case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
                floating = false;
                break;
        }
    }
    self->_active = active;
    self->_maximized = maximized;
    self->_fullscreen = fullscreen;
    self->_floating = floating;
}
static void _xdgToplevelClose(void* data, xdg_toplevel* toplevel) {
    auto self = reinterpret_cast<Decoration*>(data);
    auto& window = self->_window;

    window.dispatch(classes::EventWindowCloseRequest::kInstance);
}

static xdg_toplevel_listener _xdgToplevelListener = {
    .configure = _xdgToplevelConfigure,
    .close = _xdgToplevelClose,
};

const char* Decoration::proxyTag = "DecorationJWM";
Decoration::Decoration(WindowWayland& window):
    _window(window),
    _wm(window._windowManager) 
{
    _xdgSurface = xdg_wm_base_get_xdg_surface(_wm.xdgWm, window._waylandWindow);
    xdg_surface_add_listener(_xdgSurface, &_xdgSurfaceListener, this);
    _xdgToplevel = xdg_surface_get_toplevel(_xdgSurface);
    xdg_toplevel_add_listener(_xdgToplevel, &_xdgToplevelListener, this);
    if (_wm.decorationManager) {
        _decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(_wm.decorationManager, _xdgToplevel);
        zxdg_toplevel_decoration_v1_add_listener(_decoration, &_decorationListener, this);
        // for the love of GOD do it for me
        zxdg_toplevel_decoration_v1_set_mode(_decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
    // delay making parts until configure : )
}

void Decoration::close() {
    if (_border.surface)
        _destroyDecorations();
    if (_decoration) {
        // ???
        // zxdg_toplevel_decoration_v1_destroy(_decoration);
        _decoration = nullptr;
    }
    xdg_toplevel_destroy(_xdgToplevel);
    _xdgToplevel = nullptr;
    xdg_surface_destroy(_xdgSurface);
    _xdgSurface = nullptr;

}

void Decoration::_makePart(DecorationPart* decoration, Buffer* buf, bool opaque, int x, int y, int width, int height) {
    decoration->surface = wl_compositor_create_surface(_wm.compositor);
    wl_proxy_set_tag((wl_proxy*)decoration->surface, &proxyTag);
    wl_proxy_set_user_data((wl_proxy*)decoration->surface, this);
    decoration->subsurface = wl_subcompositor_get_subsurface(_wm.subcompositor, decoration->surface, _window._waylandWindow);
    wl_subsurface_set_position(decoration->subsurface, x, y);
    decoration->viewport = wp_viewporter_get_viewport(_wm.viewporter, decoration->surface);
    wp_viewport_set_destination(decoration->viewport, width, height);
    if (buf)
        wl_surface_attach(decoration->surface, buf->getBuffer(), 0, 0);
    
    if (opaque) {
        wl_region* region = wl_compositor_create_region(_wm.compositor);
        wl_region_add(region, 0, 0, width, height);
        wl_surface_set_opaque_region(decoration->surface, region);
        wl_surface_commit(decoration->surface);
        wl_region_destroy(region);
    } else
        wl_surface_commit(decoration->surface);
}

void Decoration::_resizeDecoration(DecorationPart* decoration, int x, int y, int width, int height) {
    if (decoration->surface) {
        wl_subsurface_set_position(decoration->subsurface, x, y);
        wp_viewport_set_destination(decoration->viewport, width, height);
        wl_surface_commit(decoration->surface);
    }
}
void Decoration::_destroyDecoration(DecorationPart* decoration) {
    if (decoration->subsurface) {
        wl_subsurface_destroy(decoration->subsurface);
    }
    if (decoration->surface)
        wl_surface_destroy(decoration->surface);
    if (decoration->viewport)
        wp_viewport_destroy(decoration->viewport);
    decoration->subsurface = nullptr;
    decoration->surface = nullptr;
    decoration->viewport = nullptr;
}

void Decoration::_destroyDecorations() {
    _destroyDecoration(&_border);
    _destroyDecoration(&_titleComp);
    _destroyDecoration(&_close);
    _destroyDecoration(&_min);
    _destroyDecoration(&_max);
}

void Decoration::_showDecorations(bool hidden) {
    // ???
    // When destroyed these get released
    _decBuffer = Buffer::createShmBuffer(_wm.shm, 1, 1, WL_SHM_FORMAT_ARGB8888);
    memcpy(_decBuffer->getData(), grey_data, 1 * sizeof(uint32_t));
    _zeroBuffer = Buffer::createShmBuffer(_wm.shm, 1, 1, WL_SHM_FORMAT_ARGB8888);
    memcpy(_zeroBuffer->getData(), zero_data, 1 * sizeof(uint32_t));
    _closeBuffer = Buffer::createShmBuffer(_wm.shm, 9, 9, WL_SHM_FORMAT_ARGB8888);
    memcpy(_closeBuffer->getData(), close_data, 9 * 9 * sizeof(uint32_t));
    _maxBuffer = Buffer::createShmBuffer(_wm.shm, 9, 9, WL_SHM_FORMAT_ARGB8888);
    memcpy(_maxBuffer->getData(), max_data, 9 * 9 * sizeof(uint32_t));
    _minBuffer = Buffer::createShmBuffer(_wm.shm, 9, 9, WL_SHM_FORMAT_ARGB8888);
    memcpy(_minBuffer->getData(), min_data, 9 * 9 * sizeof(uint32_t));
    int borderY = hidden ? DECORATION_BORDER_HIDDEN_Y : DECORATION_BORDER_Y;
    int borderHeight = hidden ? DECORATION_BORDER_HEIGHT(_window) : DECORATION_BORDER_FULL_HEIGHT(_window);
    _makePart(&_border, _zeroBuffer, false, DECORATION_BORDER_X, borderY, 
            DECORATION_BORDER_WIDTH(_window), borderHeight);
    wl_subsurface_place_below(_border.subsurface, _window._waylandWindow);
    if (!hidden) {
        _makePart(&_titleComp, _decBuffer, true, DECORATION_TITLE_X, DECORATION_TITLE_Y, 
                DECORATION_TITLE_WIDTH(_window), DECORATION_TITLE_HEIGHT);
        wl_subsurface_place_above(_titleComp.subsurface, _border.surface);
        _makePart(&_close, _closeBuffer, false, DECORATION_CLOSE_X(_window), DECORATION_CLOSE_Y, DECORATION_CLOSE_WIDTH, DECORATION_CLOSE_HEIGHT);
        _makePart(&_max, _maxBuffer, false, DECORATION_MAX_X(_window), DECORATION_MAX_Y, DECORATION_MAX_WIDTH, DECORATION_MAX_HEIGHT);
        _makePart(&_min, _minBuffer, false, DECORATION_MIN_X(_window), DECORATION_MIN_Y, DECORATION_MIN_WIDTH, DECORATION_MIN_HEIGHT);
    }

}

void Decoration::_adaptSize() {
    int borderY = _isVisible ? DECORATION_BORDER_Y : DECORATION_BORDER_HIDDEN_Y;
    int borderHeight = _isVisible ? DECORATION_BORDER_FULL_HEIGHT(_window) : DECORATION_BORDER_HEIGHT(_window);
    _resizeDecoration(&_border, DECORATION_BORDER_X, borderY, 
            DECORATION_BORDER_WIDTH(_window), borderHeight);
    if (_isVisible) {
        _resizeDecoration(&_titleComp, DECORATION_TITLE_X, DECORATION_TITLE_Y,
                DECORATION_TITLE_WIDTH(_window), DECORATION_TITLE_HEIGHT);
        _resizeDecoration(&_close, DECORATION_CLOSE_X(_window), DECORATION_CLOSE_Y, DECORATION_CLOSE_WIDTH, DECORATION_CLOSE_HEIGHT);
        _resizeDecoration(&_min, DECORATION_MIN_X(_window), DECORATION_MIN_Y, DECORATION_MIN_WIDTH, DECORATION_MIN_HEIGHT);
        _resizeDecoration(&_max, DECORATION_MAX_X(_window), DECORATION_MAX_Y, DECORATION_MAX_WIDTH, DECORATION_MAX_HEIGHT);
    }

    if (!_serverSide)
        xdg_surface_set_window_geometry(_xdgSurface, 0, _isVisible ? DECORATION_TITLE_Y : 0, 
                _window.getUnscaledWidth(), 
                (_isVisible ? DECORATION_TITLE_HEIGHT : 0) + _window.getUnscaledHeight() );
    else
        xdg_surface_set_window_geometry(_xdgSurface, 0, 0, _window.getUnscaledWidth(), _window.getUnscaledHeight());
}

bool Decoration::ownDecorationSurface(wl_surface* surface) {
    if (!surface) return false;
    return wl_proxy_get_tag((wl_proxy*)surface) == &proxyTag;
}

Decoration* Decoration::getDecorationForSurface(wl_surface* surface, DecorationFocus* focus) {
    if (ownDecorationSurface(surface)) {
        Decoration* decoration = (Decoration*)wl_proxy_get_user_data((wl_proxy*) surface);
        if (surface == decoration->_border.surface) {
            *focus = DECORATION_FOCUS_BORDER;
        } else if (surface == decoration->_titleComp.surface) {
            *focus = DECORATION_FOCUS_TITLE;
        } else if (surface == decoration->_close.surface) {
            *focus = DECORATION_FOCUS_CLOSE_BUTTON;
        } else if (surface == decoration->_min.surface) {
            *focus = DECORATION_FOCUS_MIN_BUTTON;
        } else if (surface == decoration->_max.surface) {
            *focus = DECORATION_FOCUS_MAX_BUTTON;
        }

        return decoration;
    } else {
        *focus = DECORATION_FOCUS_MAIN;
        return nullptr;
    } 
}

void Decoration::setTitlebarVisible(bool isVisible) {
    if (isVisible != _isVisible) {
        _isVisible = isVisible;
        if (isVisible) {
            _destroyDecorations();
            if (_decoration) {
                zxdg_toplevel_decoration_v1_set_mode(_decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            } else {
                _showDecorations(false);
            }
        } else {
            _destroyDecorations();
            if (_decoration) {
                zxdg_toplevel_decoration_v1_set_mode(_decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
            } else {
                _showDecorations(true);
                wl_surface_commit(_window._waylandWindow);
            }
        }
    }
}

void Decoration::setTitle(const std::string& title) {
    xdg_toplevel_set_title(_xdgToplevel, title.c_str());
    // grab a copy - unused for now but maybe a text renderer will eventually be pulled in
    _title = title;
}

void Decoration::getBorders(int& left, int& top, int& right, int& bottom) {
    if (_serverSide) {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    } else {
        left = 0;
        top = getTopSize();
        right = 0;
        bottom = 0;
    }
}

int Decoration::getTopSize() {
    if (_serverSide)
        return 0;
    if (_isVisible)
        return DECORATION_TOP_HEIGHT;
    else
        return 0;
}

void Decoration::setMinSize(int width, int height) {
    _minWidth = width;
    _minHeight = height;
}

void Decoration::constrainSize(int& width, int& height) {
    if (width < _minWidth)
        width = _minWidth;
    // hard coded for buttons
    if (width < 40)
        width = 40;
    if (height < _minHeight)
        height = _minHeight;
}
