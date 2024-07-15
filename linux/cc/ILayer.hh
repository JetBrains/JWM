#pragma once

namespace jwm {

class ILayer {
public:

    enum VSync {
        VSYNC_ADAPTIVE = -1,
        VSYNC_DISABLED = 0,
        VSYNC_ENABLED = 1,
    };

    void makeCurrent();
    virtual void makeCurrentForced();
    virtual void setVsyncMode(VSync v) = 0;
    virtual void close() = 0;


    static ILayer* _ourCurrentLayer;
};

} // namespace jwm
