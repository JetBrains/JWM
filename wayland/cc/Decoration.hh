#pragma once

#include <wayland-client.h>
#include <wayland-xdg-shell-client-protocol.h>
#include <wayland-xdg-decoration-unstable-v1-client-protocol.h>
#include <wayland-viewporter-client-protocol.h>
#include <functional>
#include "Buffer.hh"
#include <string>

#define DECORATION_WIDTH 4
#define DECORATION_TOP_HEIGHT 25

#define DECORATION_BORDER_X -(DECORATION_WIDTH)
#define DECORATION_BORDER_HIDDEN_Y -(DECORATION_WIDTH)
#define DECORATION_BORDER_Y -(DECORATION_WIDTH + DECORATION_TOP_HEIGHT)
#define DECORATION_BORDER_WIDTH(window) window.getUnscaledWidth() + DECORATION_WIDTH + DECORATION_WIDTH
#define DECORATION_BORDER_HEIGHT(window) window.getUnscaledHeight() + DECORATION_WIDTH + DECORATION_WIDTH
#define DECORATION_BORDER_FULL_HEIGHT(window) window.getUnscaledHeight() + DECORATION_TOP_HEIGHT + DECORATION_WIDTH + DECORATION_WIDTH

#define DECORATION_TITLE_X 0
#define DECORATION_TITLE_Y -(DECORATION_TOP_HEIGHT)
#define DECORATION_TITLE_WIDTH(window) window.getUnscaledWidth()
#define DECORATION_TITLE_HEIGHT DECORATION_TOP_HEIGHT

#define DECORATION_CLOSE_X(window) window.getUnscaledWidth() - 10
#define DECORATION_CLOSE_Y -20
#define DECORATION_CLOSE_WIDTH 9
#define DECORATION_CLOSE_HEIGHT 9

#define DECORATION_MAX_X(window) (DECORATION_CLOSE_X(window)) - 10
#define DECORATION_MAX_Y DECORATION_CLOSE_Y
#define DECORATION_MAX_WIDTH 9
#define DECORATION_MAX_HEIGHT 9

#define DECORATION_MIN_X(window) (DECORATION_MAX_X(window)) - 10
#define DECORATION_MIN_Y DECORATION_CLOSE_Y
#define DECORATION_MIN_WIDTH 9
#define DECORATION_MIN_HEIGHT 9

namespace jwm {
    class WindowManagerWayland;
    class WindowWayland;
    struct DecorationPart {
        wl_surface* surface = nullptr;
        wl_subsurface* subsurface = nullptr;
        wp_viewport* viewport = nullptr;
    };
    enum DecorationFocus {
        DECORATION_FOCUS_MAIN,
        DECORATION_FOCUS_BORDER,
        DECORATION_FOCUS_TITLE,
        DECORATION_FOCUS_CLOSE_BUTTON,
        DECORATION_FOCUS_MAX_BUTTON,
        DECORATION_FOCUS_MIN_BUTTON
    };
    // Creation is mapping
    // Closing is unmapping
    class Decoration {
    public:
        Decoration() = delete;
        Decoration(WindowWayland& window);
        ~Decoration();

        WindowManagerWayland& _wm;
        WindowWayland& _window;

        Buffer* _decBuffer;
        Buffer* _zeroBuffer;
        Buffer* _closeBuffer;
        Buffer* _maxBuffer;
        Buffer* _minBuffer;

        DecorationPart _border;
        DecorationPart _titleComp;

        DecorationPart _close;
        DecorationPart _max;
        DecorationPart _min;
        // May be null at runtime
        zxdg_toplevel_decoration_v1* _decoration = nullptr;

        xdg_surface* _xdgSurface = nullptr;
        xdg_toplevel* _xdgToplevel = nullptr;

        bool _serverSide = false;
        int _pendingWidth = 0;
        int _pendingHeight = 0;
        bool _oldActive = false;
        // : )
        bool _active = true;
        bool _oldMaximized = false;
        bool _maximized = false;
        bool _oldFullscreen = false;
        bool _fullscreen = false;
        bool _floating = true;
        bool _configured = false;
        // unmap and dispose
        void close();

        void _makePart(DecorationPart* part, Buffer* buf, bool opaque, int x, int y, int width, int height);
        void _resizeDecoration(DecorationPart* part, int x, int y, int width, int height);
        void _destroyDecoration(DecorationPart* part);

        void _adaptSize();
        void _destroyDecorations();
        void _showDecorations(bool hidden);

        static const char* proxyTag;
        static bool ownDecorationSurface(wl_surface* surface);
        static Decoration* getDecorationForSurface(wl_surface* surface, DecorationFocus* focus);

        std::string _title;

        void setTitle(const std::string& title);

        bool _isVisible = true;
        void setTitlebarVisible(bool isVisible);

        void getBorders(int& left, int& top, int& right, int& bottom);

        int getTopSize();

        int _minWidth = 10;
        int _minHeight = 10;
        void setMinSize(int width, int height);
        void constrainSize(int& width, int& height);


    private:
        Decoration(Decoration&& other) = delete;
        Decoration& operator=(Decoration&& other) = delete;

        Decoration(Decoration& other) = delete;
        Decoration& operator=(Decoration& other) = delete;


                
    };
}
