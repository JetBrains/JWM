#pragma once

#include <wayland-client.h>
#include <map>
#include <wayland-cursor.h>
#include "MouseCursor.hh"
#include <wayland-pointer-constraints-unstable-v1-client-protocol.h>
#include <wayland-relative-pointer-unstable-v1-client-protocol.h>
#include "Decoration.hh"

namespace jwm {
    class WindowManagerWayland;
    class WindowWayland;
    class Pointer {
    public:
        Pointer(wl_seat* seat, wl_pointer* pointer, jwm::WindowManagerWayland* wm);
        ~Pointer();

        wl_seat* _seat = nullptr;

        wl_pointer* _pointer = nullptr;
        wl_pointer* getPointer() const {
            return _pointer;
        }

        uint32_t _serial = 0;
        uint32_t getSerial() {
            return _serial;
        }

        wl_surface* _surface = nullptr;
        wl_surface* getSurface() const {
            return _surface;
        }

        WindowWayland* _focusedSurface = nullptr;
        DecorationFocus _decorationFocus = DECORATION_FOCUS_MAIN;
        WindowWayland* getFocusedSurface() const {
            return _focusedSurface;
        }

        zwp_locked_pointer_v1* _lock = nullptr;
        bool _locked = false;
        void lock();
        void unlock();
        bool isLocked() {
            return _locked;
        }

        bool _hidden = false;
        void hide();
        void unhide();
        bool isHidden() {
            return _hidden;
        }

        zwp_relative_pointer_v1* _relative = nullptr;
        bool _movement = false;
        int _absX = 0;
        int _absY = 0;
        float _dXPos = 0.0;
        float _dYPos = 0.0;
        float _dX = 0.0;
        float _dY = 0.0;

        int _mouseMask = 0;
        jwm::WindowManagerWayland& _wm;

        static wl_pointer_listener _pointerListener;

        void mouseUpdate(uint32_t x, uint32_t y, int32_t relX, int32_t relY, uint32_t mask);
        void mouseUpdateUnscaled(uint32_t x, uint32_t y, int32_t relX, int32_t relY, uint32_t mask);

        void updateHotspot(int x, int y);

        std::map<int, wl_cursor_theme*> _cursorThemes;

        wl_cursor_theme* _makeCursors(int scale);
        wl_cursor_theme* getThemeFor(int scale);
        wl_cursor* getCursorFor(int scale, jwm::MouseCursor cursor);

        jwm::MouseCursor _cursor = jwm::MouseCursor::ARROW;
        int _scale = 1;

        void setCursor(int scale, jwm::MouseCursor cursor, bool force);

        static bool ownPointer(wl_pointer* pointer);


    private:
        Pointer(const Pointer& other) = delete;
        Pointer(Pointer&&) = delete;
        Pointer& operator=(const Pointer& other) = delete;
        Pointer& operator=(Pointer&&) = delete;
    };
}
