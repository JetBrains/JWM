#include <jni.h>
#include "AppX11.hh"
#include <X11/Xresource.h>
#include <cstdlib>
#include <impl/Library.hh>
#include <impl/JNILocal.hh>
#include <X11/extensions/sync.h>
#include <X11/extensions/Xrandr.h>

jwm::AppX11 jwm::app;


float jwm::AppX11::getScale() {
    char *resourceString = XResourceManagerString(wm.display);
    XrmDatabase db;
    XrmValue value;
    char *type = NULL;

    static struct once {
        once() {
            XrmInitialize();
        }
    } once;

    if (resourceString) {
        db = XrmGetStringDatabase(resourceString);
        if (XrmGetResource(db, "Xft.dpi", "String", &type, &value)) {
            if (value.addr) {
                return atof(value.addr) / 96.f;
            }
        }
    }

    return 1.f;
}

void jwm::AppX11::init(JNIEnv* jniEnv) {
    _jniEnv = jniEnv;
}

void jwm::AppX11::start() {
    wm.runLoop();
}

void jwm::AppX11::terminate() {
    wm.terminate();
}

const std::vector<jwm::ScreenInfo>& jwm::AppX11::getScreens() {
    if (_screens.empty()) {
        Display* display = getWindowManager().getDisplay();
        XRRScreenResources* resources = XRRGetScreenResources(display, getWindowManager().getRootWindow());
        RROutput primaryOutput = XRRGetOutputPrimary(display, getWindowManager().getRootWindow());
        int count = resources->ncrtc;
        int primaryIdx = 0;

        float dpi = jwm::app.getScale();

        for (int i = 0; i < count; ++i) {
            XRRCrtcInfo* info = XRRGetCrtcInfo(display, resources, resources->crtcs[i]);
            // skip empty monitors
            if (info->width != 0) {
                for (int j = 0; j < info->noutput; ++j) {
                    RROutput output = info->outputs[j];
                    if (output == primaryOutput) {
                        primaryIdx = _screens.size();
                        break;
                    }
                }

                auto bounds = jwm::IRect::makeXYWH(info->x, info->y, info->width, info->height);

                ScreenInfo myScreenInfo = {
                    long(info->outputs[0]),
                    bounds,
                    false
                };
                _screens.push_back(myScreenInfo);
            }

            XRRFreeCrtcInfo(info);
        }
        XRRFreeScreenResources(resources);
        _screens[primaryIdx].isPrimary = true;
    }
    return _screens;
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


extern "C" JNIEXPORT jobjectArray JNICALL Java_io_github_humbleui_jwm_App__1nGetScreens(JNIEnv* env, jobject cls) noexcept {
    auto& screens = jwm::app.getScreens();
    jobjectArray array = env->NewObjectArray(screens.size(), jwm::classes::Screen::kCls, 0);    
    float scale = jwm::app.getScale();
    size_t index = 0;
    for (auto& screen : screens) {
        env->SetObjectArrayElement(array, index++, screen.asJavaObject(env));
    }

    return array;
}
