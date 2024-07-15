#pragma once

#include <ILayer.hh>

namespace jwm {
    class WindowWayland;
    class ILayerWayland: public ILayer {
    public:
        WindowWayland* fWindow = nullptr;

        virtual void attachBuffer() = 0;
        virtual void swapBuffers() = 0;
        virtual void detachBuffer();
    };
}
