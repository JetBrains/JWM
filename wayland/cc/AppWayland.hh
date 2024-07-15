#pragma once

#include "WindowManagerWayland.hh"
#include <jni.h>
#include "Types.hh"
#include <vector>
#include "impl/Library.hh"
#include "ScreenInfo.hh"
#include <wayland-client.h>

namespace jwm {
    extern class AppWayland {
    public:

        void init(JNIEnv* jniEnv);
        void start();
        void terminate();

        WindowManagerWayland& getWindowManager() {
            return wm;
        }

        JNIEnv* getJniEnv();

        JNIEnv* _jniEnv;
        WindowManagerWayland wm;

        static const char* proxyTag;

        static bool ownProxy(wl_proxy* proxy);
    } app;
}
