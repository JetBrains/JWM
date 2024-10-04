#include <algorithm>
#include <PlatformWin32.hh>
#include <AppWin32.hh>
#include <WindowWin32.hh>
#include <WindowManagerWin32.hh>
#include <Key.hh>
#include <KeyLocation.hh>
#include <KeyModifier.hh>
#include <MouseButton.hh>
#include <Log.hh>
#include <memory>
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")

jwm::WindowWin32::WindowWin32(JNIEnv *env, class WindowManagerWin32 &windowManagerWin32)
        : Window(env), _windowManager(windowManagerWin32) {

}

jwm::WindowWin32::~WindowWin32() {
    _close();
}

bool jwm::WindowWin32::init() {
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    int width = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;
    const wchar_t* caption = JWM_WIN32_WINDOW_DEFAULT_NAME;

    return _createInternal(x, y, width, height, caption);
}

void jwm::WindowWin32::recreate() {
    const wchar_t* caption = JWM_WIN32_WINDOW_DEFAULT_NAME;

    IRect rect = getWindowRect();
    int x = rect.fLeft;
    int y = rect.fTop;
    int width = rect.getWidth();
    int height = rect.getHeight();

    setFlag(Flag::IgnoreMessages);

    _destroyInternal();
    _createInternal(x, y, width, height, caption);

    setVisible(true);
    removeFlag(Flag::IgnoreMessages);
}

void jwm::WindowWin32::unmarkText() {
    JWM_VERBOSE("Request abort IME composition (unmarkText)");
    _imeResetComposition();
}

void jwm::WindowWin32::setImeEnabled(bool enabled) {
    if (enabled) {
        // Re-enabled Windows IME by associating default context.
        ImmAssociateContextEx(getHWnd(), nullptr, IACE_DEFAULT);
        JWM_VERBOSE("Enable ime text input")
    }
    else {
        // Disable Windows IME by associating 0 context.
        ImmAssociateContext(getHWnd(), nullptr);
        JWM_VERBOSE("Disable ime text input");
    }
}

void jwm::WindowWin32::setTitle(const std::wstring& title) {
    JWM_VERBOSE("Set window title '" << title << "'");
    SetWindowTextW(_hWnd, title.c_str());
}

void jwm::WindowWin32::setTitlebarVisible(bool isVisible) {
    JWM_VERBOSE("Set titlebar visible=" << isVisible << " for window 0x" << this);
    LONG_PTR lStyle = GetWindowLongPtr(_hWnd, GWL_STYLE);

    if (isVisible == true) {
        IRect windowRect = getWindowRect();

        lStyle |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        SetWindowLongPtr(_hWnd, GWL_STYLE, lStyle);

        setWindowSize(windowRect.getWidth(), windowRect.getHeight());
        JWM_VERBOSE("window shadow width '" << _windowShadowWidth << "'");

        // Reposition window to fix Windows SWP_NOMOVE still causing move in setWindowSize
        setWindowPosition(windowRect.fLeft - (int)(_windowShadowWidth * 0.5),
                          windowRect.fTop - (int)(_windowShadowHeight * 0.5));
        _windowShadowHeight = 0;
        _windowShadowWidth = 0;
    } else {
        IRect rect = getWindowRect();
        int windowWidth = rect.getWidth();
        int windowHeight = rect.getHeight();

        RECT shadowRect;
        GetWindowRect(_hWnd, &shadowRect);

        lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        SetWindowLongPtr(_hWnd, GWL_STYLE, lStyle);

        _windowShadowHeight = (shadowRect.bottom - shadowRect.top) - windowHeight;
        _windowShadowWidth = (shadowRect.right - shadowRect.left) - windowWidth;
        setContentSize(windowWidth, windowHeight);
        setWindowPosition(rect.fLeft, rect.fTop);
    }
}

void jwm::WindowWin32::setIcon(const std::wstring& iconPath) {
    JWM_VERBOSE("Set window icon '" << iconPath << "'");
    // width / height of 0 along with LR_DEFAULTSIZE tells windows to load the default icon size.
    HICON hicon = (HICON)LoadImage(NULL, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    SendMessage(_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
}

void jwm::WindowWin32::setOpacity(float opacity) {
    JWM_VERBOSE("Set window opacity'" << opacity << "'");
    LONG_PTR previousStyle = GetWindowLongPtr(_hWnd, GWL_EXSTYLE);
    float lower = 0.0;
    float upper = 1.0;
    float clamped = std::max(lower, std::min(opacity, upper));
    if (clamped == 1.0)
    {
        LONG reset = previousStyle & ~WS_EX_LAYERED;
        SetWindowLongPtr(_hWnd, GWL_EXSTYLE, reset);
        return;
    }
    LONG_PTR opacityEnabled = previousStyle |= WS_EX_LAYERED;
    SetWindowLongPtr(_hWnd, GWL_EXSTYLE, opacityEnabled);
    int adjusted = static_cast<int>(round(clamped * 255));
    SetLayeredWindowAttributes(_hWnd, RGB(0, 0, 0), adjusted, LWA_ALPHA);
}

float jwm::WindowWin32::getOpacity() {
    BYTE alpha;
    DWORD flags = LWA_ALPHA;
    BOOL hasOpacity = GetLayeredWindowAttributes(_hWnd, nullptr, &alpha, &flags);
    // GetLayeredWindowAttributes can be invoked iff the application 
    // has previously called SetLayeredWindowAttributes on the window. 
    // Otherwise it returns false as failure.
    if (!hasOpacity)
    {
        return 1.0;
    }
    return static_cast<float>(alpha / 255.0);
}

void jwm::WindowWin32::setMouseCursor(MouseCursor cursor) {
    JWM_VERBOSE("Set window cursor '" << mouseCursorToStr(cursor) << "'");

    const wchar_t* cursorName = IDC_ARROW;

    switch (cursor) {
        case MouseCursor::ARROW:
            cursorName = IDC_ARROW;
            break;
        case MouseCursor::CROSSHAIR:
            cursorName = IDC_CROSS;
            break;
        case MouseCursor::HELP:
            cursorName = IDC_HELP;
            break;
        case MouseCursor::POINTING_HAND:
            cursorName = IDC_HAND;
            break;
        case MouseCursor::IBEAM:
            cursorName = IDC_IBEAM;
            break;
        case MouseCursor::NOT_ALLOWED:
            cursorName = IDC_NO;
            break;
        case MouseCursor::WAIT:
            cursorName = IDC_WAIT;
            break;
        case MouseCursor::WIN_UPARROW:
            cursorName = IDC_UPARROW;
            break;
        case MouseCursor::RESIZE_NS:
            cursorName = IDC_SIZENS;
            break;        
        case MouseCursor::RESIZE_WE:
            cursorName = IDC_SIZEWE;
            break;
        case MouseCursor::RESIZE_NESW:
            cursorName = IDC_SIZENESW;
            break;
        case MouseCursor::RESIZE_NWSE:
            cursorName = IDC_SIZENWSE;
            break;
        
        default:
            break;
    }

    if (_hMouseCursor) {
        DestroyCursor(_hMouseCursor);
        _hMouseCursor = nullptr;
    }

    _hMouseCursor = LoadCursorW(nullptr, cursorName);
    _setMouseCursorInternal();
}

void jwm::WindowWin32::setVisible(bool value) {
    JWM_VERBOSE("Set visible=" << value << " for window 0x" << this);
    ShowWindow(_hWnd, value? SW_SHOWNA: SW_HIDE);
}

void jwm::WindowWin32::maximize() {
    JWM_VERBOSE("Maximize window 0x" << this);
    ShowWindow(_hWnd, SW_MAXIMIZE);
}

void jwm::WindowWin32::minimize() {
    JWM_VERBOSE("Minimize window 0x" << this);
    ShowWindow(_hWnd, SW_MINIMIZE);
}

void jwm::WindowWin32::restore() {
    JWM_VERBOSE("Restore window 0x" << this);
    ShowWindow(_hWnd, SW_RESTORE);
}

void jwm::WindowWin32::focus() {
    JWM_VERBOSE("Set focus on window 0x" << this);
    SetFocus(_hWnd);
}

void jwm::WindowWin32::bringToFront() {
    JWM_VERBOSE("Bring to front on window 0x" << this);
    HWND hCurrentWindow = GetForegroundWindow();
    long currentThreadId = GetWindowThreadProcessId(hCurrentWindow, NULL);
    long jwmThreadId = GetCurrentThreadId();
    AttachThreadInput(currentThreadId, jwmThreadId, true);
    SetForegroundWindow(_hWnd);
    AttachThreadInput(currentThreadId, jwmThreadId, false);
}

bool jwm::WindowWin32::isFront() {
    HWND hCurrentWindow = GetForegroundWindow();
    long currentThreadId = GetWindowThreadProcessId(hCurrentWindow, NULL);
    long jwmThreadId = GetCurrentThreadId();
    return currentThreadId == jwmThreadId;
}

void jwm::WindowWin32::requestSwap() {
    if (testFlag(Flag::HasAttachedLayer)) {
        setFlag(Flag::RequestSwap);
    }
}

void jwm::WindowWin32::requestFrame() {
    if (testFlag(Flag::HasAttachedLayer)) {
        setFlag(Flag::RequestFrame);
        _windowManager.requestFrameEvent();
    }
}

// Internal function to return simple C rect
RECT jwm::WindowWin32::_getWindowRectSimple() const {
    RECT rect;
    // Get window rect without dropshadow
    HRESULT result = DwmGetWindowAttribute(_hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    // Guard against odd situations where DwmGetWindowAttribute fails
    if (result != S_OK ||
        ((rect.right - rect.left) == 0)) {
        GetWindowRect(_hWnd, &rect);
    }
    return rect;
}

jwm::IRect jwm::WindowWin32::getWindowRect() const {
    RECT rect = _getWindowRectSimple();
    return IRect{rect.left, rect.top, rect.right, rect.bottom};
}

jwm::IRect jwm::WindowWin32::getContentRect() const {
    RECT clientRect;
    GetClientRect(_hWnd, &clientRect);
    RECT windowRect = _getWindowRectSimple();

    // Convert client area rect to screen space and
    POINT corners[] = {POINT{clientRect.left, clientRect.top}, POINT{clientRect.right, clientRect.bottom}};
    MapWindowPoints(getHWnd(), nullptr, corners, sizeof(corners)/sizeof(corners[0]));

    POINT leftTop = corners[0];
    POINT rightBottom = corners[1];

    // Build rect with client area rect relative to the window rect to handle title bar
    return IRect{leftTop.x - windowRect.left, leftTop.y - windowRect.top,
                  rightBottom.x - windowRect.left, rightBottom.y - windowRect.top};
}

void jwm::WindowWin32::setWindowPosition(int left, int top) {
    JWM_VERBOSE("Set window position left=" << left << " top=" << top);

    SetWindowPos(_hWnd, nullptr,
                 left, top,
                 0, 0,
                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void jwm::WindowWin32::setWindowSize(int width, int height) {
    JWM_VERBOSE("Set window size w=" << width << " h=" << height);

    // Calculate current shadow (reusing last stored values if set)
    RECT rect = _getWindowRectSimple();
    JWM_VERBOSE("rect left=" << rect.left << " right=" << rect.right);

    RECT rectShadow;
    GetWindowRect(_hWnd, &rectShadow);

    int shadowWidth = (rectShadow.right - rectShadow.left) - (rect.right - rect.left);
    shadowWidth = (_windowShadowWidth > shadowWidth ) ? _windowShadowWidth : shadowWidth;
    int shadowHeight = (rectShadow.bottom - rectShadow.top) - (rect.bottom - rect.top);
    shadowHeight = (_windowShadowHeight > shadowHeight ) ? _windowShadowHeight : shadowHeight;

    SetWindowPos(_hWnd, HWND_TOP,
                 0, 0,
                 width + shadowWidth,
                 height + shadowHeight,
                 SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
}

void jwm::WindowWin32::setContentSize(int width, int height) {
    RECT rect{0, 0, width, height};

    AdjustWindowRectEx(&rect, _getWindowStyle(),
                       FALSE, _getWindowExStyle());

    JWM_VERBOSE("Set content size " << "w=" << width << " h=" << height << " "
                << "(adjusted window size" << " w=" << rect.right - rect.left << " h=" << rect.bottom - rect.top << ")");

    SetWindowPos(_hWnd, HWND_TOP,
                 0, 0,
                 rect.right - rect.left,
                 rect.bottom - rect.top,
                 SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
}

jwm::ScreenWin32 jwm::WindowWin32::getScreen() const {
    HMONITOR hMonitor = MonitorFromWindow(_hWnd, MONITOR_DEFAULTTOPRIMARY);
    return ScreenWin32::fromHMonitor(hMonitor);
}

void jwm::WindowWin32::close() {
    _close();
}

LRESULT jwm::WindowWin32::processEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // https://github.com/chromium/chromium/blob/72ceeed2ebcd505b8d8205ed7354e862b871995e/ui/events/blink/web_input_event_builders_win.cc#L331
    static const float kScrollbarPixelsPerLine = 100.0f / 3.0f;

    if (testFlag(Flag::IgnoreMessages))
        return DefWindowProcW(_hWnd, uMsg, wParam, lParam);

    JNIEnv* env = getJNIEnv();

    notifyEvent(Event::SwitchContext);

    switch (uMsg) {
        // HACK: Set timer to get JWM_WM_TIMER_UPDATE_EVENT event.
        // When user hold mouse button and drag window, app enter modal loop,
        // animation is stopped. This hack allows us to get JWM_WM_TIMER_UPDATE_EVENT
        // event with minimum possible delay to repaint window and animate it.

        case WM_ENTERSIZEMOVE:
            setFlag(Flag::EnterSizeMove);
            _setFrameTimer();
            return 0;

        case WM_EXITSIZEMOVE:
            removeFlag(Flag::EnterSizeMove);
            _killFrameTimer();
            return 0;

        case WM_SIZE: {
            IRect rect = getWindowRect();
            int windowWidth = rect.getWidth();
            int windowHeight = rect.getHeight();
            int contentWidth = LOWORD(lParam);
            int contentHeight = HIWORD(lParam);

            bool minimized = wParam == SIZE_MINIMIZED;
            bool maximized = wParam == SIZE_MAXIMIZED;
            bool restored = wParam == SIZE_RESTORED && (_maximized || _minimized);

            _maximized = maximized;
            _minimized = minimized;

            JWM_VERBOSE("Size event (min="<< minimized << ", max="<< maximized << ", res=" << restored << ") "
                        << "window w=" << windowWidth << " h=" << windowHeight << " "
                        << "content w=" << contentWidth << " h=" << contentHeight);

            if (minimized)
                dispatch(classes::EventWindowMinimize::kInstance);
            if (maximized)
                dispatch(classes::EventWindowMaximize::kInstance);
            if (restored)
                dispatch(classes::EventWindowRestore::kInstance);

            JNILocal<jobject> eventWindowResize(env, classes::EventWindowResize::make(env, windowWidth, windowHeight,
                                                                                      contentWidth, contentHeight));
            dispatch(eventWindowResize.get());
            return 0;
        }

        case WM_MOVE: {
            IRect rect = getWindowRect();
            int windowLeft= rect.fLeft;
            int windowTop = rect.fTop;

            JWM_VERBOSE("Move event left=" << windowLeft << " top" << windowTop)

            JNILocal<jobject> eventMove(env, classes::EventWindowMove::make(env, windowLeft, windowTop));
            dispatch(eventMove.get());
            return 0;
        }

        case WM_ERASEBKGND:
            return true;

        case WM_TIMER: {
            if (wParam == JWM_WM_TIMER_UPDATE_EVENT) {
                // HACK: modal event loop on move/resize for focus window blocks background windows.
                // Therefore they are freeze. Here we can receive update event inside modal loop
                // to get chance to process frame event and etc.
                _windowManager.timerUpdate();
            }

            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;

            if (BeginPaint(_hWnd, &ps)) {
                dispatch(classes::EventFrame::kInstance);
                notifyEvent(Event::SwapBuffers);
                EndPaint(_hWnd, &ps);
            }

            return 0;
        }

        case WM_MOUSEMOVE: {
            _lastMousePosX = GET_X_LPARAM(lParam);
            _lastMousePosY = GET_Y_LPARAM(lParam);
            int buttons = _getMouseButtons();
            int modifiers = _getModifiers();

            int movementX = 0, movementY = 0; // TODO: impl me!
            JNILocal<jobject> eventMouseMove(env, classes::EventMouseMove::make(env, _lastMousePosX, _lastMousePosY, movementX, movementY, buttons, modifiers));
            dispatch(eventMouseMove.get());
            return 0;
        }

        case WM_MOUSEWHEEL: {
            int modifiers = _getModifiers();
            auto nativeScroll = static_cast<SHORT>(HIWORD(wParam));
            auto ticks = static_cast<float>(nativeScroll) / static_cast<float>(WHEEL_DELTA);
            auto linesPerTick = _getWheelScrollLines();
            // assume page scroll
            // https://github.com/mozilla/gecko-dev/blob/da97cbad6c9f00fc596253feb5964a8adbb45d9e/widget/windows/WinMouseScrollHandler.cpp#L891-L903
            if (linesPerTick > WHEEL_DELTA) {
                float sign =  ticks > 0.0f ? 1.0f : ticks < 0.0f ? -1.0f : 0.0f;
                JNILocal<jobject> eventMouseScroll(env, classes::EventMouseScroll::make(env, 0.0f, getContentRect().getHeight() * sign, 0.0f, 0.0f, sign, _lastMousePosX, _lastMousePosY, modifiers));
                dispatch(eventMouseScroll.get());
            } else {
                auto lines = ticks * static_cast<float>(_getWheelScrollLines());
                auto scale = _getScale();
                JNILocal<jobject> eventMouseScroll(env, classes::EventMouseScroll::make(env, 0.0f, lines * kScrollbarPixelsPerLine * scale, 0.0f, lines, 0.0f, _lastMousePosX, _lastMousePosY, modifiers));
                dispatch(eventMouseScroll.get());
            }
            return 0;
        }

        case WM_SETCURSOR: {
            if (LOWORD(lParam) == HTCLIENT) {
                _setMouseCursorInternal();
                return true;
            }

            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP: {
            bool isPressed =
                uMsg == WM_LBUTTONDOWN ||
                uMsg == WM_RBUTTONDOWN ||
                uMsg == WM_MBUTTONDOWN ||
                uMsg == WM_XBUTTONDOWN;

            int modifiers = _getModifiers();
            MouseButton button;

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
                button = MouseButton::PRIMARY;
            else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
                button = MouseButton::SECONDARY;
            else if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP)
                button = MouseButton::MIDDLE;
            else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
                button = MouseButton::BACK;
            else
                button = MouseButton::FORWARD;

            JNILocal<jobject> eventMouseButton(env, classes::EventMouseButton::make(env, button, isPressed, _lastMousePosX, _lastMousePosY, modifiers));
            dispatch(eventMouseButton.get());
            break;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_IME_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_IME_KEYUP: {
            WORD flags = HIWORD(lParam);
            bool isPressed = !(flags & KF_UP);
            bool isExtended = flags & KF_EXTENDED;
            int keycode = (int) wParam;
            int scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
            int modifiers = _getModifiers();
            auto& table = _windowManager.getKeyTable();
            auto& locations = _windowManager.getKeyLocations();
            auto& ignoreList = _windowManager.getKeyIgnoreList();

            JWM_VERBOSE("Keycode=" << keycode << " scancode=" << scancode);

            // Ignore system keys (unknown on java side)
            if (ignoreList.find(keycode) != ignoreList.end())
                break;

            auto mapping = table.find(keycode);
            Key key = mapping != table.end()? mapping->second: Key::UNDEFINED;
            KeyLocation location = KeyLocation::DEFAULT;

            // Right/numpad keys location handling
            if (isExtended || scancode == MapVirtualKeyW(VK_RSHIFT, MAPVK_VK_TO_VSC)) {
                auto locationMapping = locations.find(keycode);
                if (locationMapping != locations.end())
                    location = locationMapping->second;
            }

            JNILocal<jobject> eventKey(env, classes::EventKey::make(env, key, isPressed, modifiers, location));
            dispatch(eventKey.get());
            break;
        }

        case WM_CHAR:
        case WM_SYSCHAR: {
            if (HIGH_SURROGATE_L <= wParam && wParam <= HIGH_SURROGATE_U) {
                _highSurrogate = static_cast<wchar_t>(wParam);
            }
            else {
                jsize len = 0;
                jchar text[2];
                int modifiers = _getModifiers();

                if (modifiers & static_cast<int>(KeyModifier::CONTROL)) {
                    // If ctrl is holden, do not send text input
                    _highSurrogate = 0;
                    break;
                }

                if (LOW_SURROGATE_L <= wParam && wParam <= LOW_SURROGATE_U) {
                    if (_highSurrogate) {
                        text[len] = _highSurrogate;
                        len += 1;
                    }
                }
                else if (wParam == VK_BACK) {
                    break;
                }

                _highSurrogate = 0;

                text[len] = static_cast<wchar_t>(wParam);
                len += 1;

                JNILocal<jstring> jtext(env, env->NewString(text, len));
                JNILocal<jobject> eventTextInput(env, classes::EventTextInput::make(env, jtext.get()));
                dispatch(eventTextInput.get());
            }

            if (uMsg == WM_SYSCHAR)
                break;

            return 0;
        }

        case WM_IME_STARTCOMPOSITION:
            JWM_VERBOSE("Start IME composition");

            // Reset cache for safety
            _compositionStr.resize(0);
            _compositionPos = 0;
            _imeChangeCursorPos();
            break;

        case WM_IME_CHAR:
            return 0;

        case WM_IME_COMPOSITION: {
            int iParam = (int)lParam;

            HIMC hImc = ImmGetContext(getHWnd());

            if (!hImc)
                return false;

            if (iParam & (GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS)) {
                // Read partial composition result
                // NOTE: accessible in WM_IME_COMPOSITION or WM_IME_STARTCOMPOSITION message.
                _compositionStr = std::move(_imeGetCompositionString(hImc, GCS_COMPSTR));
                _compositionPos = ImmGetCompositionStringW(hImc, GCS_CURSORPOS, nullptr, 0);

                int selectedFrom, selectedTo;
                _imeGetCompositionStringConvertedRange(hImc, selectedFrom, selectedTo);

                if ((lParam & CS_INSERTCHAR) && (lParam & CS_NOMOVECARET)) {
                    selectedFrom = 0;
                    selectedTo = static_cast<int>(_compositionStr.length());
                }

                if (!selectedTo)
                    selectedFrom = 0;

                if (!selectedFrom && !selectedTo) {
                    selectedFrom = _compositionPos;
                    selectedTo = static_cast<int>(_compositionStr.length());
                }

                auto jcharText = reinterpret_cast<const jchar*>(_compositionStr.c_str());
                auto jlength = static_cast<jsize>(_compositionStr.length());

                JNILocal<jstring> text(env, env->NewString(jcharText, jlength));
                JNILocal<jobject> eventTextInputMarked(env, classes::EventTextInputMarked::make(env, text.get(), selectedFrom, selectedTo));
                dispatch(eventTextInputMarked.get());
            }

            if (iParam & (GCS_RESULTSTR)) {
                // If composition result, we must read its result string here.
                // NOTE: accessible in WM_IME_COMPOSITION or WM_IME_STARTCOMPOSITION message.
                _compositionStr = std::move(_imeGetCompositionString(hImc, GCS_RESULTSTR));

                auto jcharText = reinterpret_cast<const jchar*>(_compositionStr.c_str());
                auto jlength = static_cast<jsize>(_compositionStr.length());

                JNILocal<jstring> text(env, env->NewString(jcharText, jlength));
                JNILocal<jobject> eventTextInput(env, classes::EventTextInput::make(env, text.get()));
                dispatch(eventTextInput.get());
            }

            ImmReleaseContext(getHWnd(), hImc);
            return true;
        }

        case WM_IME_ENDCOMPOSITION: {
            JWM_VERBOSE("End IME composition");

            // Reset cached objects
            _compositionStr.resize(0);
            _compositionPos = 0;

            // Send commit message. This is required, for example if user suddenly closes ime window
            JNILocal<jstring> text(env, env->NewString(nullptr, 0));
            JNILocal<jobject> eventTextInput(env, classes::EventTextInput::make(env, text.get()));
            dispatch(eventTextInput.get());
            break;
        }

        case WM_IME_REQUEST:
            if (wParam == IMR_QUERYCHARPOSITION) {
                IRect IRect{};

                // If no text client set, simply break
                if (!_imeGetRectForMarkedRange(IRect))
                    break;

                // Cursor upper left corner (doc requires baseline, but its enough)
                POINT cursorPos;
                cursorPos.x = IRect.fLeft;
                cursorPos.y = IRect.fTop;
                ClientToScreen(getHWnd(), &cursorPos);

                // Area of the window (interpreted as document area (where we can place ime window))
                RECT documentArea = _getWindowRectSimple();

                // Fill lParam structure
                // its content will be read after this proc function returns
                auto* imeCharPos = reinterpret_cast<IMECHARPOSITION*>(lParam);
                imeCharPos->dwCharPos = _compositionPos;
                imeCharPos->cLineHeight = IRect.fBottom - IRect.fTop;
                imeCharPos->pt = cursorPos;
                imeCharPos->rcDocument = documentArea;
                return true;
            }
            break;
        case WM_SETFOCUS:
            JWM_VERBOSE("ON FOCUS");
            dispatch(classes::EventWindowFocusIn::kInstance);
            break;
        case WM_KILLFOCUS:
            JWM_VERBOSE("OFF FOCUS");
            dispatch(classes::EventWindowFocusOut::kInstance);
            break;

        case WM_CLOSE:
            JWM_VERBOSE("Event close");
            dispatch(classes::EventWindowCloseRequest::kInstance);
            return 0;

        default:
            break;
    }

    return DefWindowProcW(_hWnd, uMsg, wParam, lParam);
}

int jwm::WindowWin32::addEventListener(jwm::WindowWin32::Callback callback) {
    int callbackID = _getNextCallbackID();
    _eventListeners.emplace_back(callbackID, std::move(callback));
    return callbackID;
}

void jwm::WindowWin32::removeEventListener(int callbackID) {
    auto current = _eventListeners.begin();
    while (current != _eventListeners.end()) {
        if (current->first == callbackID)
            current = _eventListeners.erase(current);
        else
            ++current;
    }
}

void jwm::WindowWin32::notifyEvent(Event event) {
    for (auto& entry: _eventListeners)
        entry.second(event);
}

DWORD jwm::WindowWin32::_getWindowStyle() const {
    LONG_PTR lStyle = GetWindowLongPtr(_hWnd, GWL_STYLE);
    DWORD windowStyle;
    LongPtrToDWord(lStyle, &windowStyle);
    return windowStyle;
}

DWORD jwm::WindowWin32::_getWindowExStyle() const {
    return 0;
}

UINT jwm::WindowWin32::_getWheelScrollLines() const {
    UINT numLines;
    SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &numLines, 0);
    return numLines;
}

UINT jwm::WindowWin32::_getWheelScrollChars() const {
    UINT numLines;
    SystemParametersInfoW(SPI_GETWHEELSCROLLCHARS, 0, &numLines, 0);
    return numLines;
}

float jwm::WindowWin32::_getScale() const {
    HMONITOR hMonitor = MonitorFromWindow(_hWnd, MONITOR_DEFAULTTOPRIMARY);
    DEVICE_SCALE_FACTOR scaleFactor;
    GetScaleFactorForMonitor(hMonitor, &scaleFactor);
    if (scaleFactor == DEVICE_SCALE_FACTOR_INVALID)
        scaleFactor = JWM_DEFAULT_DEVICE_SCALE;
    return (float) scaleFactor / 100.0f;
}

int jwm::WindowWin32::_getModifiers() const {
    int modifiers = 0;

    if (GetKeyState(VK_SHIFT) & BUTTON_DOWN)
        modifiers |= static_cast<int>(KeyModifier::SHIFT);

    if (GetKeyState(VK_CONTROL) & BUTTON_DOWN)
        modifiers |= static_cast<int>(KeyModifier::CONTROL);

    if (GetKeyState(VK_MENU) & BUTTON_DOWN)
        modifiers |= static_cast<int>(KeyModifier::ALT);

    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & BUTTON_DOWN)
        modifiers |= static_cast<int>(KeyModifier::WIN_LOGO);

    return modifiers;
}

int jwm::WindowWin32::_getMouseButtons() const {
    int buttons = 0;

    if (GetKeyState(VK_LBUTTON) & BUTTON_DOWN)
        buttons |= static_cast<int>(MouseButton::PRIMARY);

    if (GetKeyState(VK_RBUTTON) & BUTTON_DOWN)
        buttons |= static_cast<int>(MouseButton::SECONDARY);

    if (GetKeyState(VK_MBUTTON) & BUTTON_DOWN)
        buttons |= static_cast<int>(MouseButton::MIDDLE);

    if (GetKeyState(VK_XBUTTON1) & BUTTON_DOWN)
        buttons |= static_cast<int>(MouseButton::BACK);

    if (GetKeyState(VK_XBUTTON2) & BUTTON_DOWN)
        buttons |= static_cast<int>(MouseButton::FORWARD);

    return buttons;
}

int jwm::WindowWin32::_getNextCallbackID() {
    return _nextCallbackID++;
}

void jwm::WindowWin32::_setFrameTimer() {
    SetTimer(_hWnd, JWM_WM_TIMER_UPDATE_EVENT, USER_TIMER_MINIMUM, nullptr);
}

void jwm::WindowWin32::_killFrameTimer() {
    KillTimer(_hWnd, JWM_WM_TIMER_UPDATE_EVENT);
}

bool jwm::WindowWin32::_createInternal(int x, int y, int w, int h, const wchar_t *caption) {
    DWORD style = _getWindowStyle();
    DWORD exStyle = _getWindowExStyle();

    HWND hWndParent = nullptr;
    HMENU hMenu = nullptr;
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    LPVOID lpParam = nullptr;

    _hWnd = CreateWindowExW(
        exStyle,
        JWM_WIN32_WINDOW_CLASS_NAME,
        caption,
        style,
        x, y,
        w, h,
        hWndParent,
        hMenu,
        hInstance,
        lpParam
    );

    if (!_hWnd) {
        JWM_LOG("Failed to init WindowWin32");
        return false;
    }

    // Set this as property to reference from message callbacks
    SetPropW(_hWnd, L"JWM", this);

    // Set default mouse cursor
    _hDefaultMouseCursor = LoadCursorW(nullptr,IDC_ARROW);

    // Register window, so manager can process its update
    _windowManager._registerWindow(*this);

    // Set active focus
    SetFocus(_hWnd);

    return true;
}

void jwm::WindowWin32::_destroyInternal() {
    // Remove window from manager and destroy OS object
    _windowManager._unregisterWindow(*this);

    if (_hMouseCursor) {
        DestroyCursor(_hMouseCursor);
        _hMouseCursor = nullptr;
    }

    if (_hWnd) {
        DestroyWindow(_hWnd);
        _hWnd = nullptr;
    }
}

void jwm::WindowWin32::_close() {
    if (_hWnd) {
        // Notify listeners that window is going to be destroyed
        notifyEvent(Event::Destroyed);
        _eventListeners.clear();

        // Native clean-up
        _destroyInternal();
    }
}

void jwm::WindowWin32::_setMouseCursorInternal() {
    if(_hMouseCursor == nullptr) {
        SetCursor(_hDefaultMouseCursor);
    } else {
        SetCursor(_hMouseCursor);
    }
}

void jwm::WindowWin32::_imeResetComposition() {
    HIMC hImc = ImmGetContext(getHWnd());

    if (!hImc)
        return;

    ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    ImmReleaseContext(getHWnd(), hImc);

    _compositionStr.resize(0);
    _compositionPos = 0;
}

void jwm::WindowWin32::_imeChangeCursorPos() const {
    IRect IRect{};

    // If no text client set, simply break
    if (!_imeGetRectForMarkedRange(IRect))
        return;

    HIMC hImc = ImmGetContext(getHWnd());

    if (!hImc)
        return;

    COMPOSITIONFORM compositionform;
    compositionform.dwStyle = CFS_FORCE_POSITION;
    compositionform.ptCurrentPos.x = IRect.fLeft;
    compositionform.ptCurrentPos.y = IRect.fBottom;

    CANDIDATEFORM candidateform;
    candidateform.dwIndex = 0;
    candidateform.dwStyle = CFS_EXCLUDE;
    candidateform.ptCurrentPos.x = IRect.fLeft;;
    candidateform.ptCurrentPos.y = IRect.fTop;
    candidateform.rcArea.left = IRect.fLeft;
    candidateform.rcArea.top = IRect.fTop;
    candidateform.rcArea.right = IRect.fRight;
    candidateform.rcArea.bottom = IRect.fBottom;

    ImmSetCompositionWindow(hImc, &compositionform);
    ImmSetCandidateWindow(hImc, &candidateform);
    ImmReleaseContext(getHWnd(), hImc);
}

void jwm::WindowWin32::_imeGetCompositionStringConvertedRange(HIMC hImc, int &selFrom, int &selTo) const {
    selFrom = selTo = 0;

    // Size in bytes without terminating null character
    DWORD sizeBytes = ImmGetCompositionStringW(hImc, GCS_COMPATTR, nullptr, 0);

    if (sizeBytes) {
        std::unique_ptr<uint8_t[]> buffer{new uint8_t[sizeBytes]};
        ImmGetCompositionStringW(hImc, GCS_COMPATTR, buffer.get(), sizeBytes);

        int start = 0;

        while (start < static_cast<int>(sizeBytes) && !(buffer[start] & ATTR_TARGET_CONVERTED))
            start += 1;

        if (start < static_cast<int>(sizeBytes)) {
            int end = start + 1;

            while (end < static_cast<int>(sizeBytes) && (buffer[end] & ATTR_TARGET_CONVERTED))
                end += 1;

            selFrom = start;
            selTo = end;
        }
    }
}

bool jwm::WindowWin32::_imeGetRectForMarkedRange(IRect &rect) const {
    // Query current cursor position
    // If composition starts, Pos will be always 0
    auto selectionStart = static_cast<int>(_compositionPos);
    auto selectionEnd = selectionStart + 0;

    JNILocal<jobject> client(fEnv, fEnv->GetObjectField(fWindow, jwm::classes::Window::kTextInputClient));
    jwm::classes::Throwable::exceptionThrown(fEnv);
    if (client.get()) {
        rect = jwm::classes::TextInputClient::getRectForMarkedRange(fEnv, client.get(), selectionStart, selectionEnd);
        return true;
    } else {
        return false;
    }
}

std::wstring jwm::WindowWin32::_imeGetCompositionString(HIMC hImc, DWORD compType) const {
    // Size in bytes without terminating null character
    DWORD sizeBytes = ImmGetCompositionStringW(hImc, compType, nullptr, 0);

    if (sizeBytes) {
        // Get actual string and copy into tmp buffer
        std::unique_ptr<uint8_t[]> buffer{new uint8_t[sizeBytes]};
        ImmGetCompositionStringW(hImc, compType, buffer.get(), sizeBytes);
        return std::wstring(reinterpret_cast<wchar_t*>(buffer.get()), sizeBytes / sizeof(wchar_t));
    }

    return std::wstring();
}

// JNI

extern "C" JNIEXPORT jlong JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nMake
        (JNIEnv* env, jclass jclass) {
    std::unique_ptr<jwm::WindowWin32> instance(new jwm::WindowWin32(env, jwm::AppWin32::getInstance().getWindowManager()));
    if (instance->init())
        return reinterpret_cast<jlong>(instance.release());
    else
        return 0;
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetTextInputEnabled
        (JNIEnv* env, jobject obj, jboolean enabled) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setImeEnabled(enabled);
    return obj;
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nUnmarkText
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->unmarkText();
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nGetWindowRect
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    jwm::IRect rect = instance->getWindowRect();
    return jwm::classes::IRect::toJava(env, rect);
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nGetContentRect
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    jwm::IRect rect = instance->getContentRect();
    return jwm::classes::IRect::toJava(env, rect);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetWindowPosition
        (JNIEnv* env, jobject obj, int left, int top) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setWindowPosition(left, top);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetWindowSize
        (JNIEnv* env, jobject obj, int width, int height) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setWindowSize(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetContentSize
        (JNIEnv* env, jobject obj, int width, int height) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setContentSize(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetTitle
        (JNIEnv* env, jobject obj, jstring title) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    const jchar* titleStr = env->GetStringChars(title, nullptr);
    jsize length = env->GetStringLength(title);
    instance->setTitle(std::wstring(reinterpret_cast<const wchar_t*>(titleStr), length));
    env->ReleaseStringChars(title, titleStr);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetTitlebarVisible
        (JNIEnv* env, jobject obj, jboolean isVisible) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setTitlebarVisible(isVisible);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetIcon
        (JNIEnv* env, jobject obj, jstring iconPath) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    const jchar* iconPathStr = env->GetStringChars(iconPath, nullptr);
    jsize length = env->GetStringLength(iconPath);
    instance->setIcon(std::wstring(reinterpret_cast<const wchar_t*>(iconPathStr), length));
    env->ReleaseStringChars(iconPath, iconPathStr);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetVisible
        (JNIEnv* env, jobject obj, jboolean isVisible) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setVisible(isVisible);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetOpacity
        (JNIEnv* env, jobject obj,float opacity) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setOpacity(opacity);
}
extern "C" JNIEXPORT float JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nGetOpacity
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    return instance->getOpacity();
}
extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nSetMouseCursor
        (JNIEnv* env, jobject obj, jint cursorId) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->setMouseCursor(static_cast<jwm::MouseCursor>(cursorId));
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nGetScreen
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    jwm::ScreenWin32 screen = instance->getScreen();
    return screen.toJni(env);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nRequestFrame
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->requestFrame();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nMaximize
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->maximize();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nMinimize
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->minimize();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nRestore
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->restore();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nFocus
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->focus();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nBringToFront
(JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->bringToFront();
}

extern "C" JNIEXPORT bool JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nIsFront
(JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    return instance->isFront();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nClose
        (JNIEnv* env, jobject obj) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    instance->close();
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_WindowWin32__1nWinSetParent
        (JNIEnv* env, jobject obj, jlong hwnd) {
    jwm::WindowWin32* instance = reinterpret_cast<jwm::WindowWin32*>(jwm::classes::Native::fromJava(env, obj));
    SetParent(instance->getHWnd(), (HWND) hwnd);
}
