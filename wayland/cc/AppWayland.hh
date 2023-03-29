#pragma once

#include "WindowManagerWayland.hh"
#include <jni.h>
#include "Types.hh"
#include <vector>
#include "impl/Library.hh"
#include "ScreenInfo.hh"

namespace jwm {
    extern class AppWayland {
    public:

        void init(JNIEnv* jniEnv);
        void start();
        void terminate();

        WindowManagerWayland& getWindowManager() {
            return wm;
        }

        JNIEnv* getJniEnv() {
            return _jniEnv;
        }

        const std::vector<ScreenInfo>& getScreens();

        float getScale();

        JNIEnv* _jniEnv;
        WindowManagerWayland wm;
        std::vector<ScreenInfo> _screens;

    } app;
}
