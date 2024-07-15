#include "Token.hh"
#include "WindowManagerWayland.hh"


static void _xdgTokenDone(void* data, xdg_activation_token_v1* token, const char* tokenStr) {
    xdg_activation_token_v1_destroy(token);
    std::string* str = reinterpret_cast<std::string*>(data);
    *str = std::string(tokenStr);
}
static xdg_activation_token_v1_listener _tokenListener = {
    .done = _xdgTokenDone
};
jwm::Token jwm::Token::make(jwm::WindowManagerWayland& wm, wl_surface* surface) {
    if (!wm.xdgActivation)
        return {};
    auto token = xdg_activation_v1_get_activation_token(wm.xdgActivation);
    if (surface)
        xdg_activation_token_v1_set_surface(token, surface);
    std::string str;
    xdg_activation_token_v1_add_listener(token, &_tokenListener, &str);
    xdg_activation_token_v1_commit(token);
    wl_display_roundtrip(wm.display);

    return {str};
}

void jwm::Token::grab(jwm::WindowManagerWayland& wm, wl_surface* surface) {
    if (!wm.xdgActivation)
        return;
    xdg_activation_v1_activate(wm.xdgActivation, token.c_str(), surface);
}
