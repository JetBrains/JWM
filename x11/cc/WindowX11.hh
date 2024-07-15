#pragma once

#include <jni.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "Window.hh"
#include "WindowManagerX11.hh"
#include "WindowX11MotifHints.hh"
#include "ILayer.hh"
#include "ScreenInfo.hh"

namespace jwm {
    class WindowX11: public jwm::Window {
    public:
        WindowX11(JNIEnv* env, WindowManagerX11& windowManager);
        ~WindowX11() override;

        void getDecorations(int& left, int& top, int& right, int& bottom);
        void getContentPosition(int& posX, int& posY);
        void setVisible(bool isVisible);
        void close();
        bool init();
        int getLeft();
        int getTop();
        int getWidth();
        int getHeight();

        void move(int left, int top);
        void resize(int width, int height);
        void requestRedraw() {
            _isRedrawRequested = true;
            _windowManager.notifyLoop();
        }
        void unsetRedrawRequest() {
            _isRedrawRequested = false;
        }
        bool isRedrawRequested() {
            return _isRedrawRequested;
        }
        void setTitle(const std::string& title);
        void setTitlebarVisible(bool isVisible);

        void maximize();
        void minimize();
        void restore();

        void setFullScreen(bool isFullScreen);
        bool isFullScreen();

        XIC getIC() const {
            return _ic;
        }
        void setCursor(jwm::MouseCursor cursor);
        void setLayer(ILayer* layer) {
            _layer = layer;
        }
        void _xSendEventToWM(Atom atom, long a, long b, long c, long d, long e) const;
        unsigned long _xGetWindowProperty(Atom property, Atom type, unsigned char** value) const;

        const ScreenInfo& getScreen();

        /**
         * _NET_WM_SYNC_REQUEST (resize flicker fix) update request counter
         */
        struct {
            uint32_t lo = 0;
            uint32_t hi = 0;
            XID counter;
        } _xsyncRequestCounter;

        int _posX = -1;
        int _posY = -1;
        int _width = -1;
        int _height = -1;
        int _WM_ADD = 1L;
        int _WM_REMOVE = 0L;
        bool _visible = false;

        bool _isRedrawRequested = false;

        WindowManagerX11& _windowManager;
        ILayer* _layer = nullptr;
        ::Window _x11Window = 0;
        XIC _ic;
    };
}
