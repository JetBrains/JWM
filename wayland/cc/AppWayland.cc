#include <jni.h>
#include "AppWayland.hh"
#include <cstdlib>
#include <wayland-client.h>
#include <cassert>
jwm::AppWayland jwm::app;


const char* jwm::AppWayland::proxyTag = "JWM";

void jwm::AppWayland::init(JNIEnv* jniEnv) {
    _jniEnv = jniEnv;
}

void jwm::AppWayland::start() {
    wm.runLoop();
}

void jwm::AppWayland::terminate() {
    wm.terminate();
}

JNIEnv* jwm::AppWayland::getJniEnv() {
    return _jniEnv;
}

bool jwm::AppWayland::ownProxy(wl_proxy* proxy) {
    if (!proxy)
        return false;
    return wl_proxy_get_tag(proxy) == &proxyTag;
}
// JNI

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_App__1nStart(JNIEnv* env, jclass jclass, jobject launcher) {
    jwm::app.init(env);
    jwm::classes::Runnable::run(env, launcher);
    jwm::app.start();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_App__1nTerminate(JNIEnv* env, jclass jclass) {
    jwm::app.terminate();
}

extern"C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    return JNI_VERSION_1_2;
}


extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_App__1nRunOnUIThread
        (JNIEnv* env, jclass cls, jobject callback) {
    jobject callbackRef = env->NewGlobalRef(callback);
    jwm::app.getWindowManager().enqueueTask([callbackRef] {
        jwm::classes::Runnable::run(jwm::app.getJniEnv(), callbackRef);
        jwm::app.getJniEnv()->DeleteGlobalRef(callbackRef);
    });
}

// how awful
extern "C" JNIEXPORT jobjectArray JNICALL Java_io_github_humbleui_jwm_App__1nGetScreens(JNIEnv* env, jobject cls) noexcept {


    jobjectArray array = env->NewObjectArray(jwm::app.wm.outputs.size(), jwm::classes::Screen::kCls, 0);
    size_t index = 0;

    for (auto& i : jwm::app.wm.outputs) {
        env->SetObjectArrayElement(array, index++, i->getScreenInfo().asJavaObject(env));
    }


    return array;
}
