package io.github.humbleui.jwm;

import java.io.*;
import java.util.concurrent.*;
import java.util.function.*;
import lombok.*;
import org.jetbrains.annotations.*;
import io.github.humbleui.jwm.*;
import io.github.humbleui.jwm.impl.*;
import io.github.humbleui.types.*;

public class WindowWin32 extends Window {
    @ApiStatus.Internal
    public WindowWin32() {
        super(_nMake());
    }

    @Override
    public Window setTextInputEnabled(boolean enabled) {
        assert _onUIThread() : "Should be run on UI thread";
        _nSetTextInputEnabled(enabled);
        return this;
    }

    @Override
    public void unmarkText() {
        assert _onUIThread() : "Should be run on UI thread";
        _nUnmarkText();
    }

    @Override
    public IRect getWindowRect() {
        assert _onUIThread() : "Should be run on UI thread";
        return _nGetWindowRect();
    }

    @Override
    public IRect getContentRect() {
        assert _onUIThread() : "Should be run on UI thread";
        return _nGetContentRect();
    }

    @Override
    public Window setWindowPosition(int left, int top) {
        assert _onUIThread() : "Should be run on UI thread";
        _nSetWindowPosition(left, top);
        return this;
    }

    @Override
    public Window setWindowSize(int width, int height) {
        assert _onUIThread() : "Should be run on UI thread";
        _nSetWindowSize(width, height);
        return this;
    }

    @Override
    public Window setContentSize(int width, int height) {
        assert _onUIThread() : "Should be run on UI thread";
        _nSetContentSize(width, height);
        return this;
    }

    @Override
    public Window setTitle(String title) {
        assert _onUIThread() : "Should be run on UI thread";
        _nSetTitle(title);
        return this;
    }

    @Override
    public Window setIcon(File icon){
        assert _onUIThread() : "Should be run on UI thread";
        _nSetIcon(icon.getAbsolutePath().toString());
        return this;
    }

    @Override
    public Window setTitlebarVisible(boolean value) {
        assert _onUIThread();
        _nSetTitlebarVisible(value);
        return this;
    }

    @Override
    public Window hideMouseCursorUntilMoved(boolean value) {
        // TODO impl me!
        return this;
    }

    @Override
    public Window lockMouseCursor(boolean value) {
        // TODO impl me!
        return this;
    }

    @Override
    public Window setVisible(boolean isVisible) {
       assert _onUIThread() : "Should be run on UI thread";
       _nSetVisible(isVisible);
       return super.setVisible(true);
    }

    @Override
    public Window setOpacity(float opacity) {
        assert _onUIThread() : "Should be run on UI thread";
        _nSetOpacity(opacity);
        return this;
    }

    @Override
    public float getOpacity() {
        assert _onUIThread() : "Should be run on UI thread";
        return _nGetOpacity();
    }


    @Override
    public Screen getScreen() {
        assert _onUIThread() : "Should be run on UI thread";
        return _nGetScreen();
    }

    @Override
    public void requestFrame() {
        if (!isClosed()) {
            App.runOnUIThread(() -> {
                if (!isClosed()) {
                    _nRequestFrame();
                }
            });
        }
    }

    public Window maximize() {
        assert _onUIThread() : "Should be run on UI thread";
        _nMaximize();
        return this;
    }

    public Window minimize() {
        assert _onUIThread() : "Should be run on UI thread";
        _nMinimize();
        return this;
    }

    public Window restore() {
        assert _onUIThread() : "Should be run on UI thread";
        _nRestore();
        return this;
    }

    @Override
    public Window setFullScreen(boolean value) {
        throw new UnsupportedOperationException("impl me!");
    }

    @Override
    public boolean isFullScreen() {
        throw new UnsupportedOperationException("impl me!");
    }

    @Override
    public Window focus() {
        assert _onUIThread() : "Should be run on UI thread";
        _nFocus();
        return this;
    }

    @Override
    public Window bringToFront() {
        assert _onUIThread() : "Should be run on UI thread";
        _nBringToFront();
        return this;
    }

    @Override
    public boolean isFront() {
        assert _onUIThread() : "Should be run on UI thread";
        return _nIsFront();
    }

    @Override
    public ZOrder getZOrder() {
        assert _onUIThread() : "Should be run on UI thread";
        return ZOrder.NORMAL;
    }

    @Override
    public Window setZOrder(ZOrder order) {
        assert _onUIThread() : "Should be run on UI thread";
        // TODO implement
        return this;
    }

    @Override
    public float getProgressBar() {
        throw new UnsupportedOperationException("impl me!");
    }

    @Override
    public Window setProgressBar(float progress) {
        throw new UnsupportedOperationException("impl me!");
    }

    @Override
    public void close() {
        assert _onUIThread() : "Should be run on UI thread";
        assert !isClosed() : "Window is already closed";
        _nClose();
        super.close();
    }

    public Window winSetParent(long hwnd) {
        assert _onUIThread() : "Should be run on UI thread";
        _nWinSetParent(hwnd);
        return this;
    }

    @ApiStatus.Internal public static native long _nMake();
    @ApiStatus.Internal public native void _nSetTextInputEnabled(boolean enabled);
    @ApiStatus.Internal public native void _nUnmarkText();
    @ApiStatus.Internal public native IRect _nGetWindowRect();
    @ApiStatus.Internal public native IRect _nGetContentRect();
    @ApiStatus.Internal public native void _nSetWindowPosition(int left, int top);
    @ApiStatus.Internal public native void _nSetWindowSize(int width, int height);
    @ApiStatus.Internal public native void _nSetContentSize(int width, int height);
    @ApiStatus.Internal public native void _nSetTitle(String title);
    @ApiStatus.Internal public native void _nSetTitlebarVisible(boolean isVisible);
    @ApiStatus.Internal public native void _nSetIcon(String iconPath);
    @ApiStatus.Internal public native void _nSetVisible(boolean isVisible);
    @ApiStatus.Internal public native void _nSetOpacity(float opacity);
    @ApiStatus.Internal public native float _nGetOpacity();
    @ApiStatus.Internal public native void _nSetMouseCursor(int cursorId);
    @ApiStatus.Internal public native Screen _nGetScreen();
    @ApiStatus.Internal public native void _nRequestFrame();
    @ApiStatus.Internal public native void _nMaximize();
    @ApiStatus.Internal public native void _nMinimize();
    @ApiStatus.Internal public native void _nRestore();
    @ApiStatus.Internal public native void _nFocus();
    @ApiStatus.Internal public native void _nBringToFront();
    @ApiStatus.Internal public native boolean _nIsFront();
    @ApiStatus.Internal public native void _nClose();
    @ApiStatus.Internal public native void _nWinSetParent(long hwnd);
}
