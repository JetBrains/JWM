#include <jni.h>
#include <AppWin32.hh>
#include "ThemeWin32.hh"
#include <VersionHelpers.h>
#include <WinUser.h>
#include <Log.hh>

using namespace jwm;

bool _isHighContrast() {
    HIGHCONTRASTA highContrast;
    highContrast.cbSize = sizeof(HIGHCONTRASTA);
    highContrast.dwFlags = 0;
    highContrast.lpszDefaultScheme = nullptr;
    bool isOk = SystemParametersInfoA(SPI_GETHIGHCONTRAST, 0, &highContrast, 0);
    if (!isOk) {
        JWM_LOG("Failed to get SystemParametersInfoA for high contrast");
        return false;
    }
    bool result = (HCF_HIGHCONTRASTON & highContrast.dwFlags) == 1;
    // JWM_VERBOSE("is HighContrast? '" << result << "'");
    return result;
}

bool _isDarkModeSupported() {
    if (_isHighContrast()) {
        return false;
    }
    // NOTE: dark mode is supported since windows 10.0.17763 but we skip strict OS version
    // check here because JWM currently targets Windows 10+. We assume it supports
    // dark mode if windows version is 10 or higher.
    return IsWindows10OrGreater();
}

bool _shouldUseDarkMode() {
    HMODULE hUxTheme = LoadLibrary(TEXT("uxtheme.dll"));
    if (!hUxTheme) {
        JWM_VERBOSE("uxtheme.dll not found.");
        return false;
    }
    FnShouldAppsUseDarkMode hFunc;
    hFunc =
        (FnShouldAppsUseDarkMode)GetProcAddress(hUxTheme, MAKEINTRESOURCEA(132));
    if (hFunc == nullptr) {
        JWM_VERBOSE("shouldAppsUseDarkMode not found.");
        FreeLibrary(hUxTheme);
        return false;
    }

    bool shouldAppsUseDarkMode;
    shouldAppsUseDarkMode = hFunc();
    FreeLibrary(hUxTheme);
    return shouldAppsUseDarkMode;
}

// JNI

extern "C" JNIEXPORT bool JNICALL Java_io_github_humbleui_jwm_Theme__1nIsHighContrast
  (JNIEnv* env, jclass jclass) {
    return _isHighContrast();
}

extern "C" JNIEXPORT bool JNICALL Java_io_github_humbleui_jwm_Theme__1nIsDark
  (JNIEnv* env, jclass jclass) {
    if(_isHighContrast()){
        return false;
    }
    if(!_isDarkModeSupported()){
        return false;
    }
    return _shouldUseDarkMode();
}
