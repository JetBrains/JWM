package io.github.humbleui.jwm;

import java.io.*;
import java.util.concurrent.*;
import java.util.function.*;
import lombok.*;
import org.jetbrains.annotations.*;
import io.github.humbleui.jwm.*;
import io.github.humbleui.jwm.impl.*;
import io.github.humbleui.types.*;

public class WindowX11 extends Window {
    @ApiStatus.Internal
    public WindowX11() {
        super(_nMake());
    }

    @Override
    public Window setTextInputEnabled(boolean enabled) {
        assert _onUIThread();
        // TODO: impl me
        return this;
    }

    @Override
    public void unmarkText() {
        assert _onUIThread();
        // TODO: impl me!
    }

    @Override 
    public IRect getWindowRect() {
        assert _onUIThread();
        return _nGetWindowRect();
    }

    @Override 
    public IRect getContentRect() {
        assert _onUIThread();
        return _nGetContentRect();
    }

    @Override
    public Window setWindowPosition(int left, int top) {
        assert _onUIThread();
        _nSetWindowPosition(left, top);
        return this;
    }

    @Override
    public Window setWindowSize(int width, int height) {
        assert _onUIThread();
        _nSetWindowSize(width, height);
        return this;
    }

    @Override
    public Window setContentSize(int width, int height) {
        assert _onUIThread();
        _nSetContentSize(width, height);
        return this;
    }

    @Override
    public Window setTitle(String title) {
        assert _onUIThread();
        try {
            _nSetTitle(title.getBytes("UTF-8"));
        } catch (UnsupportedEncodingException ignored) {}
        return this;
    }

    @Override
    public Window setIcon(File icon) {
        // TODO #95
        return this;
    }

    @Override
    public Window setTitlebarVisible(boolean value) {
        _nSetTitlebarVisible(value);
        return this;
    }

    @Override
    public Window setVisible(boolean isVisible) {
        assert _onUIThread();
        _nSetVisible(isVisible);
        return super.setVisible(true);
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
    public Window setOpacity(float opacity) {
        // TODO: impl me!
        return this;
    }

    @Override
    public float getOpacity(){
        throw new UnsupportedOperationException("impl me!");
    }

    @Override
    public Screen getScreen() {
        assert _onUIThread();
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

    @Override
    public void close() {
        assert _onUIThread() && !isClosed();
        _nClose();
        super.close();
    }

    @Override
    public Window maximize() {
        _nMaximize();
        return this;
    }

    @Override
    public Window minimize() {
        _nMinimize();
        return this;
    }

    @Override
    public Window focus() {
        assert _onUIThread();
        // TODO implement
        return this;
    }

    @Override
    public ZOrder getZOrder() {
        assert _onUIThread();
        return ZOrder.NORMAL;
    }

    @Override
    public Window setZOrder(ZOrder order) {
        assert _onUIThread();
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
    public Window restore() {
        _nRestore();
        return this;
    }

    @Override
    public Window setFullScreen(boolean value) {
        assert _onUIThread();
        _nSetFullScreen(value);
        return this;
    }

    @Override
    public boolean isFullScreen() {
        assert _onUIThread();
        return _nIsFullScreen();
    }

    @ApiStatus.Internal public static native long _nMake();
    @ApiStatus.Internal public native void _nSetVisible(boolean isVisible);
    @ApiStatus.Internal public native IRect _nGetWindowRect();
    @ApiStatus.Internal public native IRect _nGetContentRect();
    @ApiStatus.Internal public native void _nSetWindowPosition(int left, int top);
    @ApiStatus.Internal public native void _nSetWindowSize(int width, int height);
    @ApiStatus.Internal public native void _nSetMouseCursor(int cursorId);
    @ApiStatus.Internal public native void _nSetContentSize(int width, int height);
    @ApiStatus.Internal public native Screen _nGetScreen();
    @ApiStatus.Internal public native void _nRequestFrame();
    @ApiStatus.Internal public native void _nClose();
    @ApiStatus.Internal public native void _nMaximize();
    @ApiStatus.Internal public native void _nMinimize();
    @ApiStatus.Internal public native void _nRestore();
    @ApiStatus.Internal public native Screen _nSetTitle(byte[] title);
    @ApiStatus.Internal public native void _nSetTitlebarVisible(boolean isVisible);
    @ApiStatus.Internal public native void _nSetFullScreen(boolean isFullScreen);
    @ApiStatus.Internal public native boolean _nIsFullScreen();
}
