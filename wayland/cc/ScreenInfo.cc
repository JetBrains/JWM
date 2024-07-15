#include "ScreenInfo.hh"
#include "AppWayland.hh"
jobject jwm::ScreenInfo::asJavaObject(JNIEnv* env) const {
    return jwm::classes::Screen::make(env, id, isPrimary, bounds, bounds, scale);
}
