#include <iostream>
#include <jni.h>
#include "Window.hh"
#include "impl/JNILocal.hh"
#include "impl/Library.hh"

jwm::Window::~Window() {
    fEnv->DeleteGlobalRef(fWindow);
    fWindow = nullptr;
}

void jwm::Window::dispatch(jobject event) {
    if(fWindow)
        jwm::classes::Consumer::accept(fEnv, fWindow, event);
}

jobject jwm::Window::getTextInputClient() const {
    return fEnv->GetObjectField(fWindow, jwm::classes::Window::kTextInputClient);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_Window__1nInit
  (JNIEnv* env, jobject obj) {
    jwm::Window* instance = reinterpret_cast<jwm::Window*>(jwm::classes::Native::fromJava(env, obj));
    instance->fWindow = env->NewGlobalRef(obj);
}
