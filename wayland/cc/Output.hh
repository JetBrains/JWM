#pragma once

#include <wayland-client.h>
#include <cstdint>
#include "ScreenInfo.hh"

namespace jwm {
    class Output {
    public:
        Output(wl_output* output, uint32_t name);
        ~Output();

        wl_output* _output = nullptr;
        uint32_t _name;
        int scale = 1;
        int width = 0;
        int height = 0;

        ScreenInfo getScreenInfo() const;

        static wl_output_listener _outputListener;
        static void outputGeometry(void* data, wl_output* output, int x, int y, int physWidth, int physHeight,
                int subPixel, const char* make, const char* model, int transform);
        static void outputMode(void* data, wl_output* output, uint32_t flags, int width, int height, int refresh);
        static void outputDone(void* data, wl_output* output);
        static void outputScale(void* data, wl_output* output, int factor);
        static void outputName(void* data, wl_output* output, const char* name);
        static void outputDescription(void* data, wl_output* output, const char* desc);

        static Output* getForNative(wl_output* output);
        static bool ownOutput(wl_output* output);
    private:
        Output(const Output&) = delete;
        Output(Output&&) = delete;
        Output& operator=(const Output&) = delete;
        Output& operator=(Output&&) = delete;
    };
}
