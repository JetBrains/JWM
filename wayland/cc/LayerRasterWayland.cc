// JNI

#include <jni.h>
#include "impl/Library.hh"
#include "impl/RefCounted.hh"
#include "WindowWayland.hh"
#include "Buffer.hh"
#include <cstring>
#include <vector>

namespace jwm {
    class LayerRaster: public RefCounted, public ILayerWayland {
    public:
        WindowWayland* fWindow;
        size_t _width = 0, _height = 0;
        std::vector<uint8_t> _imageData;
        bool _attached = false;

        LayerRaster() = default;
        virtual ~LayerRaster() = default;

        void attach(WindowWayland* window) {
            fWindow = jwm::ref(window);
            fWindow->setLayer(this);
            if (fWindow->isConfigured()) {
                attachBuffer();
                // delay this as much as possible
                fWindow->dispatch(jwm::classes::EventWindowScreenChange::kInstance);
            }
        }

        void resize(int width, int height) {
            // god is dead
            _width = width;
            _height = height;
            _imageData = std::vector<uint8_t>(_width * _height * sizeof(uint32_t)); 
        }

        const void* getPixelsPtr() const {
            return _imageData.data();
        }

        int getRowBytes() const {

            return _width * sizeof(uint32_t);
        }

        void swapNow() {
            auto buf = Buffer::createShmBuffer(fWindow->_windowManager.shm, _width, _height, WL_SHM_FORMAT_XRGB8888);
            void* daData = buf->getData();
            size_t size = buf->getSize();
            memcpy(daData, _imageData.data(), size);
            wl_surface_attach(fWindow->_waylandWindow, buf->getBuffer(), 0, 0);
            wl_surface_damage_buffer(fWindow->_waylandWindow, 0, 0, INT32_MAX, INT32_MAX);
            wl_surface_set_buffer_scale(fWindow->_waylandWindow, fWindow->_scale);
            wl_surface_commit(fWindow->_waylandWindow);

        }
        void swapBuffers() override {
            if (_attached && fWindow->_waylandWindow) {
                // all impls that I've seen have to make a new buffer every frame.
                // God awful. Never use raster if you value performance.
                swapNow();
            }
        }

        void close() override {
            detachBuffer();
            if (fWindow) {
                fWindow->setLayer(nullptr);
                jwm::unref(&fWindow);
            }
        }

        void makeCurrentForced() override {
            ILayer::makeCurrentForced();
        }
        
        void setVsyncMode(VSync v) override {
        }

        void attachBuffer() override {
            _attached = true;
        }

        void detachBuffer() override {
            ILayerWayland::detachBuffer();
            if (_attached && fWindow && fWindow->_waylandWindow) {
                wl_surface_attach(fWindow->_waylandWindow, nullptr, 0, 0);
                // commit is not meant to be used in intermediate states
                // wl_surface_commit(fWindow->_waylandWindow);
            }
            _attached = false;
        }
    };

}
using namespace jwm;
extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nMake
        (JNIEnv* env, jclass jclass) {
    jwm::LayerRaster* instance = new jwm::LayerRaster;
    return reinterpret_cast<jlong>(instance);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_LayerRaster__1nAttach
        (JNIEnv* env, jobject obj, jobject windowObj) {
    jwm::LayerRaster* instance = reinterpret_cast<jwm::LayerRaster*>(jwm::classes::Native::fromJava(env, obj));
    jwm::WindowWayland* window = reinterpret_cast<jwm::WindowWayland*>(jwm::classes::Native::fromJava(env, windowObj));
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
