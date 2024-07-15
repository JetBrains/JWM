#include "WindowX11.hh"
#include <jni.h>
#include <memory>
#include <cstring>
#include "AppX11.hh"
#include "impl/Library.hh"
#include "impl/JNILocal.hh"
#include <X11/Xatom.h>
#include <X11/extensions/sync.h>

using namespace jwm;


WindowX11::WindowX11(JNIEnv* env, WindowManagerX11& windowManager):
    jwm::Window(env),
    _windowManager(windowManager)
{
}

WindowX11::~WindowX11() {
    close();
}

void WindowX11::setTitle(const std::string& title) {
    XChangeProperty(_windowManager.getDisplay(),
                    _x11Window,
                    _windowManager.getAtoms()._NET_WM_NAME,
                    _windowManager.getAtoms().UTF8_STRING,
                    8,
                    PropModeReplace,
                    reinterpret_cast<const unsigned char*>(title.c_str()),
                    title.length());
}

void WindowX11::setTitlebarVisible(bool isVisible) {
    MotifHints motifHints = {0};

    motifHints.flags = MOTIF_HINTS_DECORATIONS;
    motifHints.decorations = int(isVisible);

    XChangeProperty(_windowManager.getDisplay(),
                    _x11Window,
                    _windowManager.getAtoms()._MOTIF_WM_HINTS,
                    _windowManager.getAtoms()._MOTIF_WM_HINTS,
                    32,
                    PropModeReplace,
                    (unsigned char*) &motifHints,
                    5);
}

void WindowX11::close() {
    if (_x11Window) {
        _windowManager.unregisterWindow(this);
        XDestroyWindow(_windowManager.display, _x11Window);
        _x11Window = 0;
    }
}
void WindowX11::_xSendEventToWM(Atom atom, long a, long b, long c, long d, long e) const {
    XEvent event = { 0 };
    event.type = ClientMessage;
    event.xclient.window = _x11Window;
    event.xclient.format = 32; // data is 32-bit longs
    event.xclient.message_type = atom;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(_windowManager.display,
               DefaultRootWindow(_windowManager.display),
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &event);
}
unsigned long WindowX11::_xGetWindowProperty(Atom property, Atom type, unsigned char** value) const {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(_windowManager.display,
                       _x11Window,
                       property,
                       0,
                       std::numeric_limits<long>::max(),
                       false,
                       type,
                       &actualType,
                       &actualFormat,
                       &itemCount,
                       &bytesAfter,
                       value);

    return itemCount;
}

void WindowX11::maximize() {
    XWindowAttributes wa;
    XGetWindowAttributes(_windowManager.display, _x11Window, &wa);

    if (wa.map_state == IsViewable) {
        _xSendEventToWM(_windowManager._atoms._NET_WM_STATE,
                        1,
                        _windowManager._atoms._NET_WM_STATE_MAXIMIZED_HORZ,
                        _windowManager._atoms._NET_WM_STATE_MAXIMIZED_VERT,
                        0,
                        0);
    } else {
        Atom* states = nullptr;
        unsigned long count = _xGetWindowProperty(_windowManager._atoms._NET_WM_STATE,
                                                  XA_ATOM,
                                                  reinterpret_cast<unsigned char**>(&states));


        Atom missing[2] = {
            _windowManager._atoms._NET_WM_STATE_MAXIMIZED_VERT,
            _windowManager._atoms._NET_WM_STATE_MAXIMIZED_HORZ
        };
        unsigned long missingCount = 2;

        for (unsigned long i = 0;  i < count;  i++)
        {
            for (unsigned long j = 0;  j < missingCount;  j++)
            {
                if (states[i] == missing[j])
                {
                    missing[j] = missing[missingCount - 1];
                    missingCount--;
                }
            }
        }

        if (states)
            XFree(states);

        if (!missingCount)
            return;

        XChangeProperty(_windowManager.display,
                        _x11Window,
                        _windowManager._atoms._NET_WM_STATE,
                        XA_ATOM,
                        32,
                        PropModeAppend,
                        (unsigned char*) missing,
                        missingCount);
    }
    XFlush(_windowManager.display);
}

void WindowX11::minimize() {
    XIconifyWindow(_windowManager.display, _x11Window, 0);
}

void WindowX11::restore() {
    if (_windowManager._atoms._NET_WM_STATE &&
        _windowManager._atoms._NET_WM_STATE_MAXIMIZED_VERT &&
        _windowManager._atoms._NET_WM_STATE_MAXIMIZED_HORZ) {
            _xSendEventToWM(_windowManager._atoms._NET_WM_STATE,
                            0,
                            _windowManager._atoms._NET_WM_STATE_MAXIMIZED_VERT,
                            _windowManager._atoms._NET_WM_STATE_MAXIMIZED_HORZ,
                            1,
                            0);
    }
}

void WindowX11::setFullScreen(bool isFullScreen) {
    // NOTE: Largely borrowed from https://github.com/godotengine/godot/blob/f7cf9fb148140b86ee5795110373a0d55ff32860/platform/linuxbsd/x11/display_server_x11.cpp
    Display* display = _windowManager.display;

    // Should the window be exclusively full screen (i.e. block out other popups).
    // There isn't a HumbleUI setting for this, and my WM defaults to exclusive full-screen,
    // (as does Windows, as I recall) so let's assume that we want the window to be exclusively fullscreen.
    bool isExclusiveFullScreen = true;

    if (isFullScreen) { // and the window is not borderless:
        // Remove window decorations to simulate full screen
        MotifHints hints;
        Atom property;
        hints.flags = 2;
        hints.decorations = 0;
        property = XInternAtom(display, "_MOTIF_WM_HINTS", True);
        if (property != None) {
            XChangeProperty(display, _x11Window, property, property, 32, PropModeReplace, (unsigned char *)&hints, 5);
        }
    }

    XEvent xev;

    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = _x11Window;
    xev.xclient.message_type = _windowManager._atoms._NET_WM_STATE;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = isFullScreen ? _WM_ADD : _WM_REMOVE;
    xev.xclient.data.l[1] = _windowManager._atoms._NET_WM_STATE_FULLSCREEN;
    xev.xclient.data.l[2] = 0;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

    // set bypass compositor hint
    Atom bypass_compositor = XInternAtom(display, "_NET_WM_BYPASS_COMPOSITOR", True);
    unsigned long compositing_disable_on = 0; // By default, don't allow window compositing

    if (isFullScreen) {
        // NOTE: Compositor flickers. May be an issue.
        if (isExclusiveFullScreen) {
            compositing_disable_on = 1; // Force compositing to disable for efficiency
        } else {
            compositing_disable_on = 2; // Force composition on to allow pop-up windows
        }
    }

    if (bypass_compositor != None) {
        XChangeProperty(display,
                        _x11Window,
                        bypass_compositor,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (unsigned char *)&compositing_disable_on,
                        1);
    }

    XFlush(display);

    if (!isFullScreen) {
        // Reset window decorations to their previous states
        MotifHints hints;
        Atom property;
        hints.flags = 2;
        hints.decorations = 1; // Add window borders back
        property = XInternAtom(display, "_MOTIF_WM_HINTS", True);
        if (property != None) {
            XChangeProperty(display,
                            _x11Window,
                            property,
                            property,
                            32,
                            PropModeReplace,
                            (unsigned char *)&hints,
                            5);
        }
    }
}

bool WindowX11::isFullScreen() {
    // NOTE: Largely borrowed from https://github.com/godotengine/godot/blob/f7cf9fb148140b86ee5795110373a0d55ff32860/platform/linuxbsd/x11/display_server_x11.cpp
    Display* display = _windowManager.display;

    Atom type;
    int format;
    unsigned long len;
    unsigned long remaining;
    unsigned char *data = nullptr;
    bool retval = false;

    int result = XGetWindowProperty(
        display,
        _x11Window,
        _windowManager._atoms._NET_WM_STATE,
        0,
        1024,
        False,
        XA_ATOM,
        &type,
        &format,
        &len,
        &remaining,
        &data);

    if (result == Success) {
        Atom *atoms = (Atom *)data;
        for (uint64_t i = 0; i < len; i++) {
            if (atoms[i] == _windowManager._atoms._NET_WM_STATE_FULLSCREEN) {
                retval = true;
                break;
            }
        }
        XFree(data);
    }

    return retval;
}

void WindowX11::getDecorations(int& left, int& top, int& right, int& bottom) {
    unsigned long* data = nullptr;
    _xGetWindowProperty(_windowManager.getAtoms()._NET_FRAME_EXTENTS, XA_CARDINAL, reinterpret_cast<unsigned char**>(&data));
    if (data!=nullptr) {
        left   = data[0];
        top    = data[2];
        right  = data[1];
        bottom = data[3];
        XFree(data);
    } else {
        XWindowAttributes xwa;
        XGetWindowAttributes(_windowManager.display, _x11Window, &xwa);
        left   = xwa.x;
        top    = xwa.y;
        right  = 0;
        bottom = 0;
    }
}

void WindowX11::getContentPosition(int& posX, int& posY) {
    int x, y;
    ::Window child;
    XWindowAttributes wa;

    XGetWindowAttributes(_windowManager.display, _x11Window, &wa);


    XTranslateCoordinates(_windowManager.display,
                          _x11Window,
                          wa.root,
                          0, 0,
                          &x, &y,
                          &child);
    posX = x;
    posY = y;
    
}

int WindowX11::getLeft() {
    int x, y;
    getContentPosition(x, y);
    return x;
}

int WindowX11::getTop() {
    int x, y;
    getContentPosition(x, y);
    return y;
}

int WindowX11::getWidth() {
    return _width;
}

int WindowX11::getHeight() {
    return _height;
}


bool WindowX11::init()
{
    _x11Window = XCreateWindow(_windowManager.getDisplay(),
                               _windowManager.getScreen()->root,
                               0, 0,
                               800, 500,
                               0,
                               _windowManager.getX11VisualDepth(),
                               InputOutput,
                               _windowManager.getX11Visual(),
                               CWColormap | CWEventMask | CWCursor,
                               &_windowManager.getSWA()
    );

    // tell X11 we want to handle close button
    XSetWMProtocols(_windowManager.getDisplay(),
                    _x11Window,
                    &_windowManager.getAtoms().WM_DELETE_WINDOW,
                    WindowManagerX11::Atoms::PROTOCOL_COUNT);

    // IC
    {
        _ic = XCreateIC(_windowManager.getIM(),
                        XNInputStyle,
                        XIMPreeditNothing | XIMStatusNothing,
                        XNClientWindow,
                        _x11Window,
                        nullptr);

        XSetICFocus(_ic);
    }



    // XSync
    {
        XSyncValue value;
        XSyncIntToValue(&value, 0);
        _xsyncRequestCounter.counter = XSyncCreateCounter(_windowManager.getDisplay(), value);
        XChangeProperty(_windowManager.getDisplay(),
                        _x11Window,
                        _windowManager.getAtoms()._NET_WM_SYNC_REQUEST_COUNTER,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (const unsigned char*)&_xsyncRequestCounter.counter, 1);
        
    }
    _windowManager.registerWindow(this);
    return true;
}

void WindowX11::move(int left, int top) {
    _posX = left;
    _posY = top;
    if (_visible)
        XMoveWindow(_windowManager.display, _x11Window, left, top);
}

void WindowX11::resize(int width, int height) {
    _width = width;
    _height = height;
    if (_visible) {
        XResizeWindow(_windowManager.display, _x11Window, width, height);
        jwm::JNILocal<jobject> eventWindowResize(app.getJniEnv(), classes::EventWindowResize::make(app.getJniEnv(), width, height, width, height));
        dispatch(eventWindowResize.get());
    }
}

void WindowX11::setVisible(bool isVisible) {
    if (_visible != isVisible) {
        _visible = isVisible;
        if (_visible) {
            XMapWindow(_windowManager.getDisplay(), _x11Window);
            if (_posX > 0 && _posY > 0)
                move(_posX, _posY);
            if (_width > 0 && _height > 0)
                resize(_width, _height);
        } else {
            XUnmapWindow(_windowManager.getDisplay(), _x11Window);
        }
    }
}

const ScreenInfo& WindowX11::getScreen() {
    // in X11, there's no straightforward way to get screen of window.
    // instead, we should do it manually using center point of the window and calculating which monitor this point
    // belongs to.

    int centerX = getLeft() + getWidth()  / 2;
    int centerY = getTop()  + getHeight() / 2;
    for (auto& screen : jwm::app.getScreens()) {
        if (screen.bounds.isPointInside(centerX, centerY)) {
            return screen;
        }
    } 

    return *jwm::app.getScreens().begin();
}

void jwm::WindowX11::setCursor(jwm::MouseCursor cursor) {
    if (auto x11Cursor = _windowManager._cursors[static_cast<int>(cursor)]) {
        XDefineCursor(_windowManager.display, _x11Window, x11Cursor);
    } else {
        XUndefineCursor(_windowManager.display, _x11Window);
    }
}

// JNI

extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_WindowX11__1nMake
  (JNIEnv* env, jclass jclass) {
    std::unique_ptr<WindowX11> instance = std::make_unique<WindowX11>(env, jwm::app.getWindowManager());
    if (instance->init()) {
        return reinterpret_cast<jlong>(instance.release());
    }
    return 0;
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetVisible
  (JNIEnv* env, jobject obj, jboolean isVisible) {
      
    reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj))->setVisible(isVisible);
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowX11__1nGetWindowRect
  (JNIEnv* env, jobject obj) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    int left, top, right, bottom;
    instance->getDecorations(left, top, right, bottom);
    int x, y;
    instance->getContentPosition(x, y);
    return jwm::classes::IRect::toJavaXYWH(
      env,
      x-left,
      y-top,
      instance->getWidth()+left+right,
      instance->getHeight()+top+bottom
    );
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowX11__1nGetContentRect
  (JNIEnv* env, jobject obj) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    int left, top, right, bottom;
    instance->getDecorations(left, top, right, bottom);
    return jwm::classes::IRect::toJavaXYWH(
      env,
      left,
      top,
      instance->getWidth(),
      instance->getHeight()
    );
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetWindowPosition
        (JNIEnv* env, jobject obj, int left, int top) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    instance->move(left, top);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetWindowSize
        (JNIEnv* env, jobject obj, int width, int height) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    // TODO https://github.com/HumbleUI/JWM/issues/109
    instance->resize(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetContentSize
        (JNIEnv* env, jobject obj, int width, int height) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    instance->resize(width, height);
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowX11__1nGetScreen
  (JNIEnv* env, jobject obj) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    return instance->getScreen().asJavaObject(env); 
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nRequestFrame
  (JNIEnv* env, jobject obj) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    instance->requestRedraw();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nMaximize
  (JNIEnv* env, jobject obj) {
    reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj))->maximize();
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nMinimize
  (JNIEnv* env, jobject obj) {
    reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj))->minimize();
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nRestore
  (JNIEnv* env, jobject obj) {
    reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj))->restore();
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nClose
        (JNIEnv* env, jobject obj) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    instance->close();
}
extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetTitle
        (JNIEnv* env, jobject obj, jbyteArray title) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));

    jbyte* bytes = env->GetByteArrayElements(title, nullptr);
    std::string titleS = { bytes, bytes + env->GetArrayLength(title) };
    env->ReleaseByteArrayElements(title, bytes, 0);

    instance->setTitle(titleS);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetTitlebarVisible
        (JNIEnv* env, jobject obj, jboolean isVisible) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    instance->setTitlebarVisible(isVisible);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetMouseCursor
        (JNIEnv* env, jobject obj, jint idx) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));

    instance->setCursor(static_cast<jwm::MouseCursor>(idx));
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowX11__1nSetFullScreen
        (JNIEnv* env, jobject obj, jboolean isFullScreen) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    instance->setFullScreen(isFullScreen);
}

extern "C" JNIEXPORT jboolean JNICALL Java_io_github_humbleui_jwm_WindowX11__1nIsFullScreen
        (JNIEnv* env, jobject obj) {
    jwm::WindowX11* instance = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, obj));
    return instance->isFullScreen();
}

extern "C" JNIEXPORT jfloat JNICALL Java_io_github_humbleui_jwm_WindowX11__1nGetScale
        (JNIEnv* env, jobject obj) {
    return jwm::app.getScale(); 
}
