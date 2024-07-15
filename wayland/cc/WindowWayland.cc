#include "WindowWayland.hh"
#include <jni.h>
#include <memory>
#include <cstring>
#include "AppWayland.hh"
#include "impl/Library.hh"
#include "impl/JNILocal.hh"
#include <locale>
#include <codecvt>
#include <algorithm>
#include "Token.hh"

using namespace jwm;


wl_surface_listener WindowWayland::_surfaceListener = {
    .enter = WindowWayland::surfaceEnter,
    .leave = WindowWayland::surfaceLeave,
#ifdef HAVE_WAYLAND_1_22
    .preferred_buffer_scale = WindowWayland::surfacePreferredBufferScale,
    .preferred_buffer_transform = WindowWayland::surfacePreferredBufferTransform
#endif
};
 
WindowWayland::WindowWayland(JNIEnv* env, WindowManagerWayland& windowManager):
    jwm::Window(env),
    _windowManager(windowManager),
    _title("")

{
}

WindowWayland::~WindowWayland() {
    // TODO: close gets called twice?
    close();
}


void WindowWayland::setTitle(const std::string& title) {
    _title = title;
    if (_decoration)
        _decoration->setTitle(title);
}

void WindowWayland::setTitlebarVisible(bool isVisible) {
    _titlebarVisible = isVisible;
    if (_decoration)
        _decoration->setTitlebarVisible(isVisible);
}

void WindowWayland::close() {
    if (_closed) return;
    _closed = true;
    hide();
    // TODO: more destruction!
}
void WindowWayland::hide() {
    _visible = false;
    if (_layer) {
        _layer->detachBuffer();
    }
    if (_waylandWindow) {
        wl_surface_destroy(_waylandWindow);
    }
    _waylandWindow = nullptr;
    if (_decoration) {
        _decoration->close();
    }
    _decoration = nullptr;
    _windowManager.unregisterWindow(this);
}
void WindowWayland::maximize() {
    if (!_visible || !_decoration) return;
    xdg_toplevel_set_maximized(_decoration->_xdgToplevel);
}

void WindowWayland::minimize() {
    if (!_visible || !_decoration) return;
    xdg_toplevel_set_minimized(_decoration->_xdgToplevel);
}

void WindowWayland::restore() {
    // Not possible for minimize
    if (!_visible || !_decoration) return;
    xdg_toplevel_unset_maximized(_decoration->_xdgToplevel);
}

void WindowWayland::setFullScreen(bool isFullScreen) {
    if (!_visible || !_decoration) return;
    if (_decoration->_fullscreen == isFullScreen) return; 
    if (isFullScreen)
        xdg_toplevel_set_fullscreen(_decoration->_xdgToplevel, nullptr);
    else
        xdg_toplevel_unset_fullscreen(_decoration->_xdgToplevel);
}

bool WindowWayland::isFullScreen() {
    if (_decoration)
        return _decoration->_fullscreen;
    return false;
}

void WindowWayland::setLayer(ILayerWayland* layer) {
    _layer = layer;

}
void WindowWayland::getDecorations(int& left, int& top, int& right, int& bottom) {
    // impl me : )
    if (_decoration) {
        _decoration->getBorders(left, top, right, bottom);
    } else {
        left = 0;
        right = 0;
        top = 0;
        bottom = 0;
    }

}

void WindowWayland::getContentPosition(int& posX, int& posY) {
    posX = 0;
    posY = 0;
    
}

bool WindowWayland::resize(int width, int height) {
    if (width < 0 || height < 0)
        return false;
    // Width and height are in absolute pixel units, and wayland will
    // complain if you try to set a width/height that isn't a multiple of scale.
    if ((width % _scale) != 0 || (height % _scale) != 0)
        return false;

    // TODO: adapting size while visible causes issues?
    // I've noticed the frame not updating
    if (_visible)
        return false;
    _width = width / _scale;
    _height = height / _scale;
    return true;
}

int WindowWayland::getLeft() {
    int x, y;
    getContentPosition(x, y);
    return x;
}

int WindowWayland::getTop() {
    int x, y;
    getContentPosition(x, y);
    return y;
}

int WindowWayland::getWidth() {
    return getUnscaledWidth() * _scale;
}
int WindowWayland::getUnscaledWidth() {
    return _width <= 0 ? _floatingWidth : _width;
}
int WindowWayland::getHeight() {
    return getUnscaledHeight() * _scale;
}
int WindowWayland::getUnscaledHeight() {
    return _height <= 0 ? _floatingHeight : _height;
}

float WindowWayland::getScale() {
    return getIntScale();
}
int WindowWayland::getIntScale() {
    return _scale;
}
wl_cursor* WindowWayland::_getCursorFor(jwm::MouseCursor cursor) {
    if (auto ptr = _windowManager.getPointer())
        return ptr->getCursorFor(_scale, cursor);
    return nullptr;
}
bool WindowWayland::init() {
    return true;
}
bool WindowWayland::isConfigured() {
    if (_decoration)
        return _decoration->_configured;
    return false;
}
void WindowWayland::show()
{
    _waylandWindow = wl_compositor_create_surface(_windowManager.compositor);
    wl_surface_add_listener(_waylandWindow, &_surfaceListener, this);
    wl_proxy_set_tag((wl_proxy*) _waylandWindow, &AppWayland::proxyTag);
    _windowManager.registerWindow(this);
    wl_display_roundtrip(_windowManager.display);
    _decoration = new Decoration(*this);
    setTitle(_title);
    setTitlebarVisible(_titlebarVisible);
    // map
    wl_surface_commit(_waylandWindow);
    _visible = true;
}

ScreenInfo WindowWayland::getScreen() {
    if (!_outputs.empty()) {
        return _outputs.front()->getScreenInfo();
    } else {
        return {
            .id = -1,
            .bounds = jwm::IRect::makeXYWH(0, 0, getWidth(), getHeight()),
            .isPrimary = false,
            .scale = _scale
        };
    }
}
void WindowWayland::setVisible(bool isVisible) {
    if (_visible != isVisible) {
        _visible = isVisible;
        if (_visible) {
            show();
        } else {
            hide();
        }
    }
}

void jwm::WindowWayland::setCursorMaybe(jwm::MouseCursor cursor, bool force) {
    if (!_windowManager.getPointer()) return;
    _windowManager.getPointer()->setCursor(_scale, cursor, force);
}
void jwm::WindowWayland::setCursor(jwm::MouseCursor cursor) {
    setCursorMaybe(cursor, false);
}

// what do???
void jwm::WindowWayland::surfaceEnter(void* data, wl_surface* surface, wl_output* output) {
    // doesn't crash : )
    WindowWayland* self = reinterpret_cast<WindowWayland*>(data);
    
    if (auto out = Output::getForNative(output)) {
        self->_outputs.push_back(out);
        int scale = 1;
        for (auto i : self->_outputs) {
            if (i->scale > scale)
                scale = i->scale;
        }
        self->_scale = scale;
        self->_adaptSize(self->getUnscaledWidth(), self->getUnscaledHeight());
    }
}
bool jwm::WindowWayland::isNativeSelf(wl_surface* surface) {
    if (!_waylandWindow) return false;
    return surface == _waylandWindow;
}
bool jwm::WindowWayland::ownSurface(wl_surface* surface) {
    return AppWayland::ownProxy((wl_proxy*) surface);
}
void jwm::WindowWayland::surfaceLeave(void* data, wl_surface* surface, wl_output* output) {
    auto self = reinterpret_cast<WindowWayland*>(data);
    
    if (auto out = Output::getForNative(output)) {
        auto it = std::find(self->_outputs.begin(), self->_outputs.end(), out);

        if (it != self->_outputs.end())
            self->_outputs.erase(it);

        int scale = 1;
        for (auto i : self->_outputs) {
            if (i->scale > scale)
                scale = i->scale;
        }
        self->_scale = scale;
        self->_adaptSize(self->getUnscaledWidth(), self->getUnscaledHeight());
    }
}
void jwm::WindowWayland::surfacePreferredBufferScale(void* data, wl_surface* surface, int factor) {
    WindowWayland* self = (WindowWayland*) data;
    if (factor < 1) {
        return;
    }
    self->_scale = factor;
    // do I pinky promise here?
    // yes : )
    self->_adaptSize(self->getUnscaledWidth(), self->getUnscaledHeight());
}
void jwm::WindowWayland::surfacePreferredBufferTransform(void* data, wl_surface* surface, uint32_t transform) {}

static void frameCallbackDone(void* data, wl_callback* cb, uint32_t cb_data) {
    auto self = reinterpret_cast<WindowWayland*>(data);
    self->_adaptSize(self->_newWidth, self->_newHeight);
}
wl_callback_listener jwm::WindowWayland::_frameCallback = {
    .done = frameCallbackDone
};
void jwm::WindowWayland::_adaptSize(int newWidth, int newHeight) {
    using namespace classes;
    if (!isConfigured()) {
        return;
    }
    if (newWidth == _width && newHeight == _height && _scale == _oldScale) return;
    _width = newWidth;
    _height = newHeight;
    int scaledWidth = _width * _scale;
    int scaledHeight = _height * _scale;
    jwm::JNILocal<jobject> eventWindowResize(
               app.getJniEnv(),
               EventWindowResize::make(
                        app.getJniEnv(),
                        scaledWidth,
                        scaledHeight,
                        scaledWidth,
                        scaledHeight
                   )
            );
    dispatch(eventWindowResize.get());
    // In Java Wayland doesn't actually cause a frame:
    // however decorFrameCommit will cause a redraw anyway.
    // Not doing it in wayland lets me not cause an exception on hide.
}

void jwm::WindowWayland::lockCursor(bool locked) {
    auto pointer = _windowManager.getPointer();
    if (!pointer) return;
    if (pointer->getFocusedSurface() == this) {
        if (locked)
            pointer->lock();
        else
            pointer->unlock();
    }
}
void jwm::WindowWayland::hideCursor(bool hidden) {
    auto pointer = _windowManager.getPointer();
    if (!pointer) return;
    if (pointer->getFocusedSurface() == this) {
        if (hidden)
            pointer->hide();
        else
            pointer->unhide();
    }
}

void jwm::WindowWayland::focus() {
    if (!_waylandWindow) return;
    auto token = Token::make(_windowManager, _waylandWindow);
    token.grab(_windowManager, _waylandWindow);
}
// JNI

extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nMake
  (JNIEnv* env, jclass jclass) {
    std::unique_ptr<WindowWayland> instance = std::make_unique<WindowWayland>(env, jwm::app.getWindowManager());
    if (instance->init()) {
        return reinterpret_cast<jlong>(instance.release());
    }
    return 0;
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nSetVisible
  (JNIEnv* env, jobject obj, jboolean isVisible) {
      
    reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj))->setVisible(isVisible);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nSetContentSize
    (JNIEnv* env, jobject obj, jint width, jint height) {
    reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj))->resize(width, height);    
}
extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nGetScreen
    (JNIEnv* env, jobject obj) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    return instance->getScreen().asJavaObject(env);
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nGetWindowRect
  (JNIEnv* env, jobject obj) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
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

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nGetContentRect
  (JNIEnv* env, jobject obj) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
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


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nRequestFrame
  (JNIEnv* env, jobject obj) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    instance->requestRedraw();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nMaximize
  (JNIEnv* env, jobject obj) {
    reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj))->maximize();
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nMinimize
  (JNIEnv* env, jobject obj) {
    reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj))->minimize();
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nRestore
  (JNIEnv* env, jobject obj) {
    reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj))->restore();
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nClose
        (JNIEnv* env, jobject obj) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    instance->close();
}
extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nSetTitle
        (JNIEnv* env, jobject obj, jstring title) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));

    const jchar* bytes = env->GetStringChars(title, nullptr);
    jsize length = env->GetStringLength(title);
    std::u16string thingie = {reinterpret_cast<const char16_t*>(bytes), static_cast<size_t>(length)};

    std::string titleS = std::wstring_convert<
            std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(thingie);
    env->ReleaseStringChars(title, bytes);
    instance->setTitle(titleS);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nSetTitlebarVisible
        (JNIEnv* env, jobject obj, jboolean isVisible) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    instance->setTitlebarVisible(isVisible);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nSetMouseCursor
        (JNIEnv* env, jobject obj, jint idx) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));

    instance->setCursor(static_cast<jwm::MouseCursor>(idx));
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nSetFullScreen
        (JNIEnv* env, jobject obj, jboolean isFullScreen) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    instance->setFullScreen(isFullScreen);
}

extern "C" JNIEXPORT jboolean JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nIsFullScreen
        (JNIEnv* env, jobject obj) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    return instance->isFullScreen();
}

extern "C" JNIEXPORT jfloat JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nGetScale
        (JNIEnv* env, jobject obj) {
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    return instance->getScale();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nLockMouseCursor
        (JNIEnv* env, jobject obj, jboolean locked) 
{
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    instance->lockCursor(locked);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nHideMouseCursor
        (JNIEnv* env, jobject obj, jboolean hidden)
{
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    instance->hideCursor(hidden);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWayland__1nFocus
    (JNIEnv* env, jobject obj)
{
    jwm::WindowWayland* instance = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, obj));
    instance->focus();
}
