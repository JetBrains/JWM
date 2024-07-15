#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <vector>
#include "Types.hh"
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "MouseCursor.hh"
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <list>
#include "Output.hh"
#include <xkbcommon/xkbcommon.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include "Keyboard.hh"
#include "Pointer.hh"
#include <memory>
#include <wayland-pointer-constraints-unstable-v1-client-protocol.h>
#include <wayland-relative-pointer-unstable-v1-client-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>
#include <wayland-xdg-decoration-unstable-v1-client-protocol.h>
#include <wayland-viewporter-client-protocol.h>
#include <wayland-xdg-activation-v1-client-protocol.h>

namespace jwm {
    class WindowWayland;
    class WindowManagerWayland {
    public:
        WindowManagerWayland();

        void runLoop();
        void terminate();

        void registerWindow(WindowWayland* window);
        void unregisterWindow(WindowWayland* window);
        
        void _processCallbacks();
        void _processKeyboard();
        void notifyLoop();
        void enqueueTask(const std::function<void()>& task);

        WindowWayland* getWindowForNative(wl_surface* surface);

        static wl_registry_listener _registryListener;

        static void registryHandleGlobal(void* data, wl_registry *registry,
                uint32_t name, const char* interface, uint32_t version);
        static void registryHandleGlobalRemove(void* data, wl_registry *registry,
                uint32_t name);

        static wl_seat_listener _seatListener;

        static void seatCapabilities(void* data, wl_seat* seat, uint32_t capabilities);
        static void seatName(void* data, wl_seat* seat, const char* name);

        static wl_data_source_listener _sourceListener;
        static wl_data_offer_listener _offerListener;
        static wl_data_device_listener _deviceListener;

        ByteBuf getClipboardContents(const std::string& type);
        std::vector<std::string> getClipboardFormats();

        wl_display* display = nullptr;
        wl_registry* registry = nullptr;
        wl_shm* shm = nullptr;
        wl_compositor* compositor = nullptr;
        wl_data_device_manager* deviceManager = nullptr;
        // no multiseat?
        wl_seat* seat = nullptr;
        zwp_pointer_constraints_v1* pointerConstraints = nullptr;
        zwp_relative_pointer_manager_v1* relativePointerManager = nullptr;
        xdg_wm_base* xdgWm = nullptr;
        wp_viewporter* viewporter = nullptr;
        zxdg_decoration_manager_v1* decorationManager = nullptr;
        wl_subcompositor* subcompositor = nullptr;
        xdg_activation_v1* xdgActivation = nullptr;
        std::unique_ptr<Pointer> _pointer = nullptr;
        Pointer* getPointer() const {
                return _pointer.get();
        }
        std::unique_ptr<Keyboard> _keyboard = nullptr;
        wl_data_device* dataDevice = nullptr;
        wl_data_source* currentSource = nullptr;
        wl_data_offer* currentOffer = nullptr;
        uint32_t getMouseSerial() const {
                if (_pointer)
                        return _pointer->getSerial();
                return 0;
        }
        uint32_t getKeyboardSerial() const {
                if (_keyboard)
                        return _keyboard->getSerial();
                return 0;
        }
        xkb_state* getXkbState() const {
                if (_keyboard)
                        return _keyboard->getState();
                return nullptr;
        }


        EGLDisplay _eglDisplay = EGL_NO_DISPLAY;
        std::list<Output*> outputs;

        bool _runLoop;
        int notifyFD = -1;
        std::atomic_bool notifyBool{false};

        std::list<WindowWayland*> _windows;
        std::map<std::string, ByteBuf> _myClipboardContents;
        std::map<std::string, ByteBuf> _myClipboardSource;
        std::list<std::string> _currentMimeTypes;
        const char* vendor = nullptr;

        wl_surface* getCursorSurface() const {
                if (_pointer)
                        return _pointer->getSurface();
                return nullptr;
        }
        WindowWayland* getFocusedSurface() const {
                if (_pointer)
                        return _pointer->getFocusedSurface();
                return nullptr;
        }
        // Is holding all cursors in memory a good idea?
        wl_cursor_image* _cursors[static_cast<int>(jwm::MouseCursor::COUNT)];


        std::mutex _taskQueueLock;
        std::condition_variable _taskQueueNotify;
        std::queue<std::function<void()>> _taskQueue;


        void setClipboardContents(std::map<std::string, ByteBuf>&& c);
    };
}
