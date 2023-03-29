#include <algorithm>
#include <iostream>
#include <jni.h>
#include "impl/Library.hh"
#include "impl/RefCounted.hh"
#include "WindowWayland.hh"
#include <GL/gl.h>

namespace jwm {

    class LayerGL: public RefCounted, public ILayer {
    public:
        WindowWayland* fWindow;
        GLXContext _context = nullptr;
        using glXSwapIntervalEXT_t = void (*)(Display*, GLXDrawable, int);   
        glXSwapIntervalEXT_t _glXSwapIntervalEXT;

        LayerGL() = default;
        virtual ~LayerGL() = default;

        void attach(WindowWayland* window) {
            if (window->_windowManager.getVisualInfo() == nullptr) {
                throw std::runtime_error("layer not supported");             
            }

            fWindow = jwm::ref(window);
            fWindow->setLayer(this);

            if (_context == nullptr) {
                _context = glXCreateContext(window->_windowManager.getDisplay(),
                                            window->_windowManager.getVisualInfo(),
                                            nullptr,
                                            true);
                                    
            }
            
            makeCurrentForced();

            _glXSwapIntervalEXT = reinterpret_cast<glXSwapIntervalEXT_t>(glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXSwapIntervalEXT")));
            setVsyncMode(VSYNC_ADAPTIVE);
        }

        void setVsyncMode(VSync v) override {

            if (_glXSwapIntervalEXT) {
                _glXSwapIntervalEXT(fWindow->_windowManager.getDisplay(),
                                    fWindow->_x11Window,
                                    v);
            }
        }

        void resize(int width, int height) {
            glClearStencil(0);
            glClearColor(0, 0, 0, 255);
            glStencilMask(0xffffffff);
            glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glViewport(0, 0, width, height);
        }

        void swapBuffers() {
            glXSwapBuffers(fWindow->_windowManager.getDisplay(), fWindow->_x11Window);
        }

        void close() override {
            glXDestroyContext(fWindow->_windowManager.getDisplay(), _context);
            jwm::unref(&fWindow);
        }

        void makeCurrentForced() override {
            ILayer::makeCurrentForced();
            glXMakeCurrent(fWindow->_windowManager.getDisplay(),
                          fWindow->_x11Window,
                          _context);
        }
    };

} // namespace jwm

// JNI

extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_LayerGL__1nMake
  (JNIEnv* env, jclass jclass) {
    jwm::LayerGL* instance = new jwm::LayerGL();
    return reinterpret_cast<jlong>(instance);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nAttach
  (JNIEnv* env, jobject obj, jobject windowObj) {
    try {
        jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
        jwm::WindowWayland* window = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, windowObj));
        instance->attach(window);
    } catch (const std::exception& e) {
        jwm::classes::Throwable::throwLayerNotSupportedException(env, "Failed to init OpenGL");
    }
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nReconfigure
  (JNIEnv* env, jobject obj, jint width, jint height) {
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nResize
  (JNIEnv* env, jobject obj, jint width, jint height) {
    jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
    instance->resize(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nMakeCurrent
  (JNIEnv* env, jobject obj) {
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nSwapBuffers
  (JNIEnv* env, jobject obj) {
    jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
    instance->swapBuffers();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerGL__1nClose
  (JNIEnv* env, jobject obj) {
    jwm::LayerGL* instance = reinterpret_cast<jwm::LayerGL*>(jwm::classes::Native::fromJava(env, obj));
    instance->close();
}
