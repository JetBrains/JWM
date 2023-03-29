// JNI

#include <jni.h>
#include "impl/Library.hh"
#include "impl/RefCounted.hh"
#include "WindowX11.hh"

namespace jwm {
    class LayerRaster: public RefCounted, public ILayer {
    public:
        WindowX11* fWindow;
        size_t _width = 0, _height = 0;
        XImage* _xImage = nullptr;
        GC _graphicsContext;
        VSync _vsync = VSYNC_ENABLED;

        /**
         * Using raw pointer here because XImage frees this buffer on XDestroyImage.
         */
        uint8_t* _imageData = nullptr;

        LayerRaster() = default;
        virtual ~LayerRaster() = default;

        void attach(WindowX11* window) {
            fWindow = jwm::ref(window);
            fWindow->setLayer(this);

            Display* d = fWindow->_windowManager.getDisplay();
            _graphicsContext = DefaultGC(d, DefaultScreen(d));
        }

        void resize(int width, int height) {
            Display* d = fWindow->_windowManager.getDisplay();
            _width = width;
            _height = height;
            if (_xImage) {
                XDestroyImage(_xImage);
            }
            _imageData = new uint8_t[width * height * sizeof(uint32_t)];
            _xImage = XCreateImage(d, CopyFromParent, DefaultDepth(d, DefaultScreen(d)), ZPixmap, 0, (char*)_imageData, width, height, 32, 0);
            XInitImage(_xImage);
            _xImage->byte_order = _xImage->bitmap_bit_order = LSBFirst;
        }

        const void* getPixelsPtr() const {
            return _imageData;
        }

        int getRowBytes() const {
            return _width * sizeof(uint32_t);
        }

        void swapBuffers() {
            XPutImage(fWindow->_windowManager.getDisplay(), fWindow->_x11Window, _graphicsContext, _xImage, 0, 0, 0, 0, _width, _height);
        }

        void close() override {
            if (_xImage) {
                XDestroyImage(_xImage);
                _xImage = nullptr;
            }
            jwm::unref(&fWindow);
        }

        void makeCurrentForced() override {
            ILayer::makeCurrentForced();
        }
        
        void setVsyncMode(VSync v) override {
            _vsync = v;
        }
    };
}


extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nMake
        (JNIEnv* env, jclass jclass) {
    jwm::LayerRaster* instance = new jwm::LayerRaster;
    return reinterpret_cast<jlong>(instance);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nAttach
        (JNIEnv* env, jobject obj, jobject windowObj) {
    jwm::LayerRaster* instance = reinterpret_cast<jwm::LayerRaster*>(jwm::classes::Native::fromJava(env, obj));
    jwm::WindowX11* window = reinterpret_cast<jwm::WindowX11*>(jwm::classes::Native::fromJava(env, windowObj));
    instance->attach(window);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nReconfigure
        (JNIEnv* env, jobject obj) {
    
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nResize
        (JNIEnv* env, jobject obj, jint width, jint height) {
    jwm::LayerRaster* instance = reinterpret_cast<jwm::LayerRaster*>(jwm::classes::Native::fromJava(env, obj));
    instance->resize(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nSwapBuffers
        (JNIEnv* env, jobject obj) {
    jwm::LayerRaster* instance = reinterpret_cast<jwm::LayerRaster*>(jwm::classes::Native::fromJava(env, obj));
    instance->swapBuffers();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nClose
        (JNIEnv* env, jobject obj) {
    jwm::LayerRaster* instance = reinterpret_cast<jwm::LayerRaster*>(jwm::classes::Native::fromJava(env, obj));
    instance->close();
}

extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nGetPixelsPtr
        (JNIEnv* env, jobject obj) {
    jwm::LayerRaster* instance = reinterpret_cast<jwm::LayerRaster*>(jwm::classes::Native::fromJava(env, obj));
    return reinterpret_cast<jlong>(instance->getPixelsPtr());
}

extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nGetRowBytes
        (JNIEnv* env, jobject obj) {
    jwm::LayerRaster* instance = reinterpret_cast<jwm::LayerRaster*>(jwm::classes::Native::fromJava(env, obj));
    return static_cast<jint>(instance->getRowBytes());
} 