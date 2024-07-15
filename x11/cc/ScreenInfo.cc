#include "ScreenInfo.hh"
#include "AppX11.hh"


jobject jwm::ScreenInfo::asJavaObject(JNIEnv* env) const {
    return jwm::classes::Screen::make(env, id, isPrimary, bounds, bounds, jwm::app.getScale());
}
