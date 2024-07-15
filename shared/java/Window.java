package io.github.humbleui.jwm;

import java.io.File;
import java.util.concurrent.*;
import java.util.function.*;
import io.github.humbleui.jwm.impl.*;
import io.github.humbleui.types.*;
import lombok.*;
import org.jetbrains.annotations.*;

public abstract class Window extends RefCounted implements Consumer<Event> {
    @ApiStatus.Internal
    public MouseCursor _lastCursor = MouseCursor.ARROW;

    @ApiStatus.Internal @Getter
    public Layer _layer = null;

    @ApiStatus.Internal @Getter
    public Consumer<Event> _eventListener = null;

    @ApiStatus.Internal @Getter
    public TextInputClient _textInputClient = null;

    @ApiStatus.Internal
    public Window(long ptr) {
        super(ptr);
        _nInit();
    }

    /**
     * <p>Set window event listener, which accepts window specific events.</p>
     *
     * @param eventListener listener to set
     * @return              this
     */
    @NotNull @Contract("-> this")
    public Window setEventListener(@Nullable Consumer<Event> eventListener) {
        _eventListener = eventListener;
        return this;
    }

    /**
     * <p>Set window text input client used for complex text input and IME support.</p>
     *
     * @param client        text input client
     * @return              this
     *
     * @see <a href="https://github.com/HumbleUI/JWM/blob/main/examples/java/io/github/humbleui/jwm/examples/PanelTextInput.java">Text input example</a>
     */
    @NotNull @Contract("-> this")
    public Window setTextInputClient(@Nullable TextInputClient client) {
        _textInputClient = client;
        return this;
    }

    @NotNull @Contract("-> this")
    public Window setLayer(@Nullable Layer layer) {
        assert _onUIThread() : "Should be run on UI thread";
        if (_layer != null) {
            _layer.close();
            _layer = null;
        }
        if (layer != null) {
            _layer = layer;
            layer.attach(this);
            // accepting this immediately causes crashes on wayland
            if (Platform.CURRENT != Platform.WAYLAND)
                accept(EventWindowScreenChange.INSTANCE);
        }
        return this;
    }

    public abstract float getScale();
    /**
     * <p>Enables complex text input on this window.</p>
     * <p>Passed value `true` or `false` enables or disables complex text input and IME on this window respectively.</p>
     *
     * @param enabled       flag to enabled text input
     * @return              this
     *
     * @see <a href="https://github.com/HumbleUI/JWM/blob/main/examples/java/io/github/humbleui/jwm/examples/PanelTextInput.java">Text input example</a>
     */
    @NotNull @Contract("-> this")
    public abstract Window setTextInputEnabled(boolean enabled);

    /**
     * <p>Unmark currently marked and edited text region.</p>
     *
     * <p>Call to this function terminates current IME editing mode and causes close of the
     * system IME window. This function must be called in general if user finishes
     * text editing because of context change, clicking somewhere else, opening another tab and etc.</p>
     *
     * @see <a href="https://github.com/HumbleUI/JWM/blob/main/examples/java/io/github/humbleui/jwm/examples/PanelTextInput.java">Text input example</a>
     */
    public abstract void unmarkText();

    /**
     * <p>Get window position and size on the screen as UI rect.</p>
     *
     * <p>Returned rect contains position and size of the whole window,
     * including window frame decoration.</p>
     * <p>Position is in unified global screen coordinate space with 1:1 pixel mapping.
     * Origin (0,0) is in general top-left corner of the primary desktop screen.</p>
     *
     * <p>Note: on Linux platform has some limitations, due to window decorations
     * info is specific for concrete desktop environment.</p>
     *
     * @return          UI window rect
     */
    public abstract IRect getWindowRect();

    /**
     * <p>Get window content position and size in the window as UI rect.</p>
     *
     * <p>Returned rect contains the position and size of the window internal content area,
     * also know as client area on Windows, framebuffer in glfw and etc.</p>
     * <p>Returned rect position is relative to left-top window position, if window
     * has top bar decoration, returned position will be non-zero, otherwise it will be the exact (0,0).</p>
     * <p>Returned rect size is actual size of the content area. This size must be used for rendering
     * purposes, if you want to create graphics layer and draw something with correct framebuffer metrics.</p>
     *
     * <p>Note: on Linux platform has some limitations, due to window decorations
     * info is specific for concrete desktop environment.</p>
     *
     * @return          UI content rect
     */
    public abstract IRect getContentRect();

    /**
     * <p>Get window content position and size on the screen as UI rect.</p>
     * <p>This function has the same effect as {@link #getContentRect()}, but returned rect has screen relative coordinates.</p>
     *
     * @return          UI content rect
     */
    public IRect getContentRectAbsolute() {
        IRect windowRect = getWindowRect();
        return getContentRect().offset(windowRect._left, windowRect._top);
    }

    /**
     * <p>Set window position on the screen.</p>
     *
     * <p>Affects window rect and positions window top-left corner, which includes window frame and decorations.</p>
     * <p>For precise window content area positioning you must take into
     * account window decorations and correct this function input.</p>
     * <p>Position is in unified global screen coordinate space with 1:1 pixel mapping.
     * Origin (0,0) is in general top-left corner of the primary desktop screen.</p>
     *
     * @param left      window top-left corner x position in pixels
     * @param top       window top-left corner y position in pixels
     * @return          this
     *
     * @see <a href="https://github.com/HumbleUI/JWM/blob/main/docs/Getting%20Started.md">Getting Started</a>
     */
    @NotNull @Contract("-> this")
    public abstract Window setWindowPosition(int left, int top);

    /**
     * <p>Set window size on the screen.</p>
     *
     * <p>Affects content and window rect, which includes window frame and decorations.</p>
     * <p>For precise window content area sizing you must take into
     * account window decorations and correct this function input.</p>
     *
     * @param width     window rect width in pixels
     * @param height    window rect height in pixels
     * @return          this
     *
     * @see <a href="https://github.com/HumbleUI/JWM/blob/main/docs/Getting%20Started.md">Getting Started</a>
     */
    @NotNull @Contract("-> this")
    public abstract Window setWindowSize(int width, int height);

    /**
     * <p>Set window content area size.</p>
     *
     * <p>Affects content and window rect, which includes window frame and decorations.</p>
     *
     * @param width     window content area width in pixels
     * @param height    window content area height in pixels
     * @return          this
     *
     * @see <a href="https://github.com/HumbleUI/JWM/blob/main/docs/Getting%20Started.md">Getting Started</a>
     */
    @NotNull @Contract("-> this")
    public abstract Window setContentSize(int width, int height);

    /**
     * <p>Set window title.</p>
     *
     * <p>If window has decorations, the title will be displayed in the top bar.</p>
     *
     * @param title     window title string to set
     * @return          this
     */
    @NotNull @Contract("-> this")
    public abstract Window setTitle(@NotNull String title);

    /**
     * <p>Set window icon from file on the disk.</p>
     *
     * <p>Icon displayed in the top-bar and menu on Linux and Windows.</p>
     *
     * @param icon      icon file on the disk
     * @return          this
     */
    @NotNull @Contract("-> this")
    public abstract Window setIcon(File icon);

    /**
     * <p>Completely removes the titlebar from the window including buttons and title.</p>
     *
     * <p>See platform windows for platform specific configuration.</p>
     *
     * @param isVisible visibility flag value
     * @return this
     */
    @NotNull @Contract("-> this")
    public abstract Window setTitlebarVisible(boolean isVisible);

    /**
     * <p>Set system mouse cursor, displayed on the window.</p>
     *
     * @param cursor    system cursor type
     * @return          this
     */
    @NotNull @Contract("-> this")
    public Window setMouseCursor(MouseCursor cursor) {
        assert _onUIThread() : "Should be run on UI thread";
        if (cursor != _lastCursor) {
            _lastCursor = cursor;
            _nSetMouseCursor(cursor.ordinal());
        }
        return this;
    }

    /**
     * <p>Hides mouse cursor until moved</p>
     *
     * @return  this
     */
    @NotNull @Contract("-> this")
    public Window hideMouseCursorUntilMoved() {
        return hideMouseCursorUntilMoved(true);
    }

    /**
     * <p>Hides mouse cursor until moved. Call with `false` to unhide</p>
     *
     * @param value  true to hide, false to unhide
     * @return       this
     */
    @NotNull @Contract("_ -> this")
    public abstract Window hideMouseCursorUntilMoved(boolean value);

    /**
     * <p>Locks and hides mouse cursor. Call with `false` to unlock and unhide.</p>
     *
     * @param value  true to lock/hide, false to unlock/unhide
     * @return       this
     */
    @NotNull @Contract("_ -> this")
    public abstract Window lockMouseCursor(boolean value);

    /**
     * <p>Set window visibility.</p>
     *
     * <p>If passed `true`, the window will be visible for the user.</p>
     * <p>If passed `false`, the window and its menu icon will be invisible for the user.</p>
     *
     * <p>Window visibility might be useful, if you want to fully configure window
     * and display only final result to the user. Also hidden windows might be useful,
     * if you have complex graphics layer setup, and you want to display window and draw something
     * without lag and delay for graphics context setup.</p>
     *
     * @param isVisible visibility flag value
     * @return          this
     */
    @NotNull @Contract("-> this")
    public Window setVisible(boolean isVisible) {
        if (isVisible)
            accept(EventWindowScreenChange.INSTANCE);
        return this;
    }

    /**
     * Sets window opacity [0.0, 1.0]. If the opacity is outside the range,
     * it will be clamped to 0.0 to 1.0.
     * 
     * @param opacity   0f for fully transparent, 1f for fully opaque
     * @return          this
     */
    @NotNull @Contract("-> this")
    public abstract Window setOpacity(float opacity);

    /**
     * <p>Get window opacity value.</p>
     *
     * @return          opacity value in range [0.0, 1.0]
     */
    public abstract float getOpacity();

    /**
     * <p>Get window screen, where currently this window is located.</p>
     *
     * @return          screen object where this window located
     */
    public abstract Screen getScreen();

    /**
     * <p>Request frame event for rendering for this window.</p>
     *
     * <p>Requested frame event won't be sent immediately. But, its is guaranty to be sent to sync with next v-sync.</p>
     * <p>Call this function after previous frame event if you want to draw some animation and etc.</p>
     */
    public abstract void requestFrame();

    /**
     * <p>Expands the window to take up all of the space of the desktop except taskbars, docks, etc.</p>
     *
     * <p>On Windows title-bar, the single square icon [] has the same behaviour.</p>
     * <p>On macOS title-bar, the yellow circle has the same behaviour.</p>
     *
     * @return          this
     */
    public abstract Window maximize();

    /**
     * <p>Hides the window to the system taskbar or dock.</p>
     *
     * <p>On Windows title-bar, the underline icon _ has the same behaviour.</p>
     * <p>On macOS title-bar, the green circle has the same behaviour.</p>
     *
     * @return          this
     */
    public abstract Window minimize();

    /**
     * <p>Cancels the effect of {@link #minimize()} and {@link #maximize()} functions.</p>
     *
     * <p>On Windows title-bar, the double square icon []] has the same behaviour.</p>
     * <p>On macOS title-bar, the yellow circle has the same behaviour.</p>
     *
     * @return          this
     */
    public abstract Window restore();

    public abstract Window setFullScreen(boolean value);

    public abstract boolean isFullScreen();

    /**
     * Focus current window
     *
     * @return  this
     */
    public abstract Window focus();

    /**
     * <p>Bring window to front-most position in operating-system.</p>
     * <p>This can bring the window out of the system's backgrounded apps.</p>
     *
     * @return  this
     */
    public abstract Window bringToFront();

    /**
     * Checks if window is currently the front-most one in the parent operating system.
     *
     * @return  boolean
     */
    public abstract boolean isFront();

    /**
     * @return  current Z order
     */
    public abstract ZOrder getZOrder();

    /**
     * <p>Makes window float always on top.</p>
     * 
     * @return  this
     */
    public abstract Window setZOrder(ZOrder order);

    /**
     * @return The current progress bar value for this window
     */
    public abstract float getProgressBar();

    /**
     * <p>Changes the progress bar depending on the platform.</p>
     * <p>macOS changes the value for the entire App.</p>
     * <p>Windows is not implemented for now</p>
     * <p>Linux is not implemented for now</p>
     *
     * @param progress The new progress bar value for this window
     * @return this
     */
    @NotNull @Contract("-> this")
    public abstract Window setProgressBar(float progress);

    /**
     * <p>Feed an event to Window’s listener. Same as getEventListener().accept(Event)</p>
     */
    public void accept(Event e) {
        if (_layer != null) {
            if (e instanceof EventWindowScreenChange) {
                _layer.reconfigure();
            } else if (e instanceof EventWindowResize) {
                EventWindowResize ee = (EventWindowResize) e;
                _layer.resize(ee.getContentWidth(), ee.getContentHeight());
            } else if (e instanceof EventFrame) {
                _layer.frame();
            }
        }

        if (_eventListener != null) {
            try {
                _eventListener.accept(e);
            } catch (Exception ex) {
                Log.log(ex);
            }
        }

        if (e instanceof EventWindowScreenChange) {
            accept(new EventWindowResize(this));
        } else if (e instanceof EventWindowResize && Platform.CURRENT != Platform.X11) {
            accept(EventFrame.INSTANCE);
        }
    }

    /**
     * <p>Close window and release its internal resources.</p>
     * <p>Must be last window object method call. After this method window is in released state.</p>
     */
    @Override
    public void close() {
        assert _onUIThread() : "Should be run on UI thread";
        assert !isClosed() : "Window is already closed";
        setLayer(null);
        Consumer<Event> eventListener = _eventListener;
        setEventListener(null);
        setTextInputClient(null);
        App._windows.remove(this);
        super.close();
        if (eventListener != null)
            eventListener.accept(EventWindowClose.INSTANCE);
    }

    @ApiStatus.Internal public static boolean _onUIThread() {
        return App._onUIThread();
    }

    @ApiStatus.Internal public native void _nInit();
    @ApiStatus.Internal public abstract void _nSetMouseCursor(int cursorIdx);
}
