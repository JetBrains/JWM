#pragma once
#include <Window.hh>
#include <MouseCursor.hh>
#include <PlatformWin32.hh>
#include <ScreenWin32.hh>
#include <impl/Library.hh>
#include <functional>
#include <utility>
#include <atomic>
#include <bitset>
#include <vector>
#include <string>
#include <jni.h>

namespace jwm {
    class WindowWin32 final: public Window {
    public:
        enum class Event {
            SwitchContext,
            SwapBuffers,
            EnableVsync,
            DisableVsync,
            Destroyed
        };

        enum class Flag: size_t {
            EnterSizeMove = 1,
            RequestFrame = 2,
            RequestSwap = 3,
            HasAttachedLayer = 4,
            RecreateForNextLayer = 5,
            IgnoreMessages = 6,
            HasLayerGL = 7,
            HasLayerD3D = 8,
            HasLayerRaster = 9,
            Max = 10
        };

        using Callback = std::function<void(Event)>;
        static const int BUTTON_DOWN = 0x8000;
        static const UINT HIGH_SURROGATE_L = 0xd800;
        static const UINT HIGH_SURROGATE_U = 0xdbff;
        static const UINT LOW_SURROGATE_L = 0xdc00;
        static const UINT LOW_SURROGATE_U = 0xdfff;

    public:
        explicit WindowWin32(JNIEnv* env, class WindowManagerWin32& windowManagerWin32);
        ~WindowWin32() override;
        bool init();
        void recreate();
        void unmarkText();
        void setImeEnabled(bool enabled);
        void setTitle(const std::wstring& title);
        void setTitlebarVisible(bool isVisible);
        void setIcon(const std::wstring& iconPath);
        void setOpacity(float opacity);
        float getOpacity();
        bool isHighContrast();
        void setMouseCursor(MouseCursor cursor);
        void setVisible(bool value);
        void maximize();
        void minimize();
        void restore();
        void focus();
        void bringToFront();
        bool isFront();
        void requestSwap();
        void requestFrame();
        IRect getWindowRect() const;
        IRect getContentRect() const;
        void setWindowPosition(int left, int top);
        void setWindowSize(int width, int height);
        void setContentSize(int width, int height);
        ScreenWin32 getScreen() const;
        void close();
        LRESULT processEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    public:
        int addEventListener(Callback callback);
        void removeEventListener(int callbackID);
        void notifyEvent(Event event);
        void setFlag(Flag flag) { _flags.set(static_cast<size_t>(flag), true); }
        void removeFlag(Flag flag) { _flags.set(static_cast<size_t>(flag), false); }
        bool testFlag(Flag flag) { return _flags.test(static_cast<size_t>(flag)); }
        JNIEnv* getJNIEnv() const { return fEnv; }
        HWND getHWnd() const { return _hWnd; }

    private:
        DWORD _getWindowStyle() const;
        DWORD _getWindowExStyle() const;
        UINT _getWheelScrollLines() const;
        UINT _getWheelScrollChars() const;
        float _getScale() const;
        int _getModifiers() const;
        int _getMouseButtons() const;
        int _getNextCallbackID();
        void _setFrameTimer();
        void _killFrameTimer();
        bool _createInternal(int x, int y, int w, int h, const wchar_t* caption);
        void _destroyInternal();
        void _close();
        void _setMouseCursorInternal();
        void _imeResetComposition();
        void _imeChangeCursorPos() const;
        void _imeGetCompositionStringConvertedRange(HIMC hImc, int &selFrom, int &selTo) const;
        bool _imeGetRectForMarkedRange(IRect& rect) const;
        std::wstring _imeGetCompositionString(HIMC hImc, DWORD compType) const;
        RECT _getWindowRectSimple() const;

    private:
        friend class WindowManagerWin32;

        std::vector<std::pair<int, Callback>> _eventListeners;
        std::bitset<static_cast<size_t>(Flag::Max)> _flags;
        std::wstring _compositionStr;
        LONG _compositionPos = 0;

        class WindowManagerWin32& _windowManager;

        HWND _hWnd = nullptr;
        HCURSOR _hMouseCursor = nullptr;
        HCURSOR _hDefaultMouseCursor = nullptr;
        int _lastMousePosX = 0;
        int _lastMousePosY = 0;
        bool _minimized = false;
        bool _maximized = false;
        int _nextCallbackID = 0;
        wchar_t _highSurrogate = 0;
        int _windowShadowWidth = 0;
        int _windowShadowHeight = 0;
    };
}
