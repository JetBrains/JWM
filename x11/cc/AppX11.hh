#pragma once

#include "WindowManagerX11.hh"
#include <jni.h>
#include "Types.hh"
#include <vector>
#include "impl/Library.hh"
#include "ScreenInfo.hh"

namespace jwm {
    extern class AppX11 {
    public:

        void init(JNIEnv* jniEnv);
        void start();
        void terminate();

        WindowManagerX11& getWindowManager() {
            return wm;
        }

        JNIEnv* getJniEnv() {
            return _jniEnv;
        }

        const std::vector<ScreenInfo>& getScreens();

        float getScale();

        JNIEnv* _jniEnv;
        WindowManagerX11 wm;
        std::vector<ScreenInfo> _screens;

    } app;
}