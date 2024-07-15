package io.github.humbleui.jwm;

import lombok.*;
import java.lang.Thread;
import java.lang.reflect.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.function.*;
import org.jetbrains.annotations.*;
import io.github.humbleui.jwm.impl.*;

public class App {
    @ApiStatus.Internal
    public static List<Window> _windows = Collections.synchronizedList(new ArrayList<Window>());
    @ApiStatus.Internal
    public static long _uiThreadId;

    /**
     * <p>Start the JWM application, blocking until completion.</p>
     * <p>User init should be handled inside the `launcher` callback (on the main thread).</p>
     *
     * <p>This method must be the first method called in the JWM library.</p>
     * <p>After this method call, library API can be safely accessed for creating windows and querying system info.</p>
     */
    public static void start(@NotNull Runnable launcher) {
        Library.load();
        ClassLoader cl = Thread.currentThread().getContextClassLoader();
        _nStart(() -> {
            Thread t = Thread.currentThread();
            t.setContextClassLoader(cl);
            _uiThreadId = t.getId();
            Log.setVerbose("true".equals(System.getenv("JWM_VERBOSE")));
            long t0 = System.currentTimeMillis();
            Log.setLogger((s) -> System.out.println("[ " + (System.currentTimeMillis() - t0) + " ] " + s));
            launcher.run();
        });
    }

    /**
     * <p>Make new native platform-specific window.</p>
     * <p>Note: must be called only after or inside {@link #start(Runnable)} successful method call.</p>
     *
     * @return          new window instance
     */
    @NotNull @SneakyThrows
    public static Window makeWindow() {
        assert _onUIThread() : "Should be run on UI thread";
        Window window;
        if (Platform.CURRENT == Platform.WINDOWS)
            window = new WindowWin32();
        else if (Platform.CURRENT == Platform.MACOS)
            window = new WindowMac();
        else if (Platform.CURRENT == Platform.X11)
            window = new WindowX11();
        else if (Platform.CURRENT == Platform.WAYLAND)
            window = new WindowWayland();
        else
            throw new RuntimeException("Unsupported platform: " + Platform.CURRENT);
        _windows.add(window);
        return window;
    }

    /**
     * <p>Request application terminate.</p>
     * <p>Note: must be called only after or inside {@link #start(Runnable)} successful method call.</p>
     */
    public static void terminate() {
        assert _onUIThread() : "Should be run on UI thread";
        _nTerminate();
    }

    /**
     * <p>Enqueue callback function to run on application UI thread.</p>
     * <p>This is the only method, which can be access from any thread if other is not specified.</p>
     * <p>Use this method to access UI thread and safely perform user-specific work and access JWM API.</p>
     *
     * @param callback  function for execution on UI thread
     */
    public static void runOnUIThread(Runnable callback) {
        if (_onUIThread())
            callback.run();
        else
            _nRunOnUIThread(callback);
    }

    /**
     * <p>Get desktop environment screens configurations.</p>
     * <p>Note: must be called only after or inside {@link #start(Runnable)} successful method call.</p>
     *
     * @return          list of desktop screens
     */
    public static Screen[] getScreens() {
        assert _onUIThread() : "Should be run on UI thread";
        return _nGetScreens();
    }

    /**
     * <p>Get desktop environment primary screen info.</p>
     * <p>Note: must be called only after or inside {@link #start(Runnable)} successful method call.</p>
     *
     * @return          primary desktop screen
     */
    @Nullable
    public static Screen getPrimaryScreen() {
        assert _onUIThread() : "Should be run on UI thread";
        if (Platform.CURRENT == Platform.WAYLAND)
            return null;
        for (Screen s: getScreens())
            if (s.isPrimary())
                return s;
        return null;
    }

    public static void openSymbolsPalette() {
        assert _onUIThread() : "Should be run on UI thread";
        if (Platform.CURRENT == Platform.MACOS)
            _nOpenSymbolsPalette();
        else
            throw new RuntimeException("Not supported on " + Platform.CURRENT);
    }

    @ApiStatus.Internal public static boolean _onUIThread() {
        return _uiThreadId == Thread.currentThread().getId();
    }

    @ApiStatus.Internal public static native void _nStart(Runnable launcher);
    @ApiStatus.Internal public static native void _nTerminate();
    @ApiStatus.Internal public static native Screen[] _nGetScreens();
    @ApiStatus.Internal public static native void _nRunOnUIThread(Runnable callback);
    @ApiStatus.Internal public static native void _nOpenSymbolsPalette();
}
