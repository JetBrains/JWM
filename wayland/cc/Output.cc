#include "Output.hh"
#include "AppWayland.hh"


using namespace jwm;

wl_output_listener Output::_outputListener = {
    .geometry = Output::outputGeometry,
    .mode = Output::outputMode,
    .done = Output::outputDone,
    .scale = Output::outputScale,
    .name = Output::outputName,
    .description = Output::outputDescription
};
Output::Output(wl_output* output, uint32_t name):
    _output(output),
    _name(name)
    {
        wl_output_add_listener(output, &_outputListener, this);
        wl_proxy_set_tag((wl_proxy*) output, &AppWayland::proxyTag);
    }
Output::~Output()
{
    if (_output)
        wl_output_release(_output);
}

ScreenInfo Output::getScreenInfo() const {
    return {
        .id = _name,
        .bounds = jwm::IRect::makeXYWH(0, 0, width, height),
        .isPrimary = false,
        .scale = scale
    };
}
void Output::outputGeometry(void* data, wl_output* output, int x, int y, int physWidth, int physHeight,
            int subPixel, const char* make, const char* model, int transform) {}
void Output::outputMode(void* data, wl_output* output, uint32_t flags, int width, int height, int refresh) {
    Output* self = reinterpret_cast<Output*>(data);
    self->width = width;
    self->height = height;
}
void Output::outputDone(void* data, wl_output* output) {}
void Output::outputScale(void* data, wl_output* output, int factor) {
    Output* self = reinterpret_cast<Output*>(data);
    self->scale = factor;
}
void Output::outputName(void* data, wl_output* output, const char* name) {

}
void Output::outputDescription(void* data, wl_output* output, const char* desc) {}

Output* Output::getForNative(wl_output* output) {
    if (!output) return nullptr;

    if (ownOutput(output)) {
        return reinterpret_cast<Output*>(wl_output_get_user_data(output));
    }
    return nullptr;
}

bool Output::ownOutput(wl_output* output) {
    return AppWayland::ownProxy((wl_proxy*) output);
}
