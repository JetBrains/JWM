package io.github.humbleui.jwm.examples;

import io.github.humbleui.jwm.*;
import io.github.humbleui.jwm.skija.*;
import io.github.humbleui.skija.*;
import io.github.humbleui.types.*;

import java.util.*;
import java.util.concurrent.*;
import java.util.function.*;
import java.util.stream.*;
import java.io.File;

public class Example implements Consumer<Event> {
    public static int PADDING = 10;
    public static int COLS = 4, ROWS = 3;
    public static final KeyModifier MODIFIER = Platform.CURRENT == Platform.MACOS ? KeyModifier.MAC_COMMAND : KeyModifier.CONTROL;
    public static Font FONT12 = new Font(FontMgr.getDefault().matchFamilyStyleCharacter(null, FontStyle.NORMAL, null, "↑".codePointAt(0)), 12);
    public static Font FONT24 = new Font(FontMgr.getDefault().matchFamilyStyle(null, FontStyle.NORMAL), 24);
    public static Font FONT48 = new Font(FontMgr.getDefault().matchFamilyStyle(null, FontStyle.BOLD), 48);

    public float lastScale = 1f;
    public PanelTextInput panelTextInput;
    public PanelScreens panelScreens;
    public PanelLegend panelLegend;
    public PanelMouse panelMouse;
    public PanelAnimation panelAnimation;
    public PanelMouseCursors panelMouseCursors;
    public PanelRendering panelRendering;
    public PanelEvents panelEvents;
    public PanelTheme panelTheme;
    public PanelTouch panelTouch;
    public boolean initialized = false;

    public Window window;

    public boolean paused = true;

    public Options progressBars = new Options("Default", "0%", "50%", "100%", "Indeterminate");

    public Example() {
        window = App.makeWindow();
        window.setEventListener(this);

        panelTextInput = new PanelTextInput(window);
        panelScreens = new PanelScreens(window);
        panelLegend = new PanelLegend(window);
        panelMouse = new PanelMouse(window);
        panelAnimation = new PanelAnimation(window);
        panelMouseCursors = new PanelMouseCursors(window);
        panelRendering = new PanelRendering(window);
        panelEvents = new PanelEvents(window);
        panelTheme = new PanelTheme(window);
        panelTouch = new PanelTouch(window);

        var scale = window.getScale();
        int count = App._windows.size() - 1;
        Screen screen = App.getScreens()[(count / 5) % App.getScreens().length];
        IRect bounds = screen.getWorkArea();
        // IRect bounds = new IRect(0, 0, 100, 100);

        window.setTitle("JWM Window #" + count);
        if (window instanceof WindowMac windowMac) {
            windowMac.setSubtitle("Window Subtitle");
            windowMac.setRepresentedFilename("macos/cc/WindowMac.mm");
        }

        panelScreens.setTitleStyle(panelScreens.titleStyles.get(count));

        window.setWindowSize(bounds.getWidth() / 2, bounds.getHeight() / 2);
        switch (count % 5) {
            case 0 -> window.setWindowPosition(bounds.getLeft() + bounds.getWidth() / 4, bounds.getTop() + bounds.getHeight() / 4);
            case 1 -> window.setWindowPosition(bounds.getLeft(), bounds.getTop());
            case 2 -> window.setWindowPosition(bounds.getLeft() + bounds.getWidth() / 2, bounds.getTop());
            case 3 -> window.setWindowPosition(bounds.getLeft(), bounds.getTop() + bounds.getHeight() / 2);
            case 4 -> window.setWindowPosition(bounds.getLeft() + bounds.getWidth() / 2, bounds.getTop() + bounds.getHeight() / 2);
        }

        switch (Platform.CURRENT) {
            case WINDOWS -> {
                window.setIcon(new File("examples/dashboard/resources/windows.ico"));
            }
            case MACOS -> {
                window.setIcon(new File("examples/dashboard/resources/macos.icns"));
            }
        }

        window.setVisible(true);
        initialized = true;
    }

    public void paint(Canvas canvas, int width, int height) {
        float scale = window.getScreen().getScale();
        PADDING = (int) (10 * scale);
        int panelWidth = (width - (COLS + 1) * PADDING) / COLS;
        int panelHeight = (height - (ROWS + 1) * PADDING) / ROWS;
        if (panelWidth <= 0 || panelHeight <= 0)
            return;

        if (lastScale != scale) {
            FONT12.setSize(12 * scale);
            FONT24.setSize(24 * scale);
            FONT48.setSize(48 * scale);
        }

        canvas.clear(0xFF264653);
        int canvasCount = canvas.save();

        // First row
        panelMouse.paint        (canvas, PADDING + (panelWidth + PADDING) * 0, PADDING + (panelHeight + PADDING) * 0, panelWidth, panelHeight, scale);
        panelTouch.paint     (canvas, PADDING + (panelWidth + PADDING) * 1, PADDING + (panelHeight + PADDING) * 0, panelWidth, panelHeight, scale);
        panelTextInput.paint    (canvas, PADDING + (panelWidth + PADDING) * 2, PADDING + (panelHeight + PADDING) * 0, panelWidth, panelHeight, scale);
        panelMouseCursors.paint (canvas, PADDING + (panelWidth + PADDING) * 3, PADDING + (panelHeight + PADDING) * 0, panelWidth, panelHeight, scale);

        // Second row
        panelScreens.paint      (canvas, PADDING + (panelWidth + PADDING) * 0, PADDING + (panelHeight + PADDING) * 1, panelWidth, panelHeight, scale);
        panelAnimation.paint    (canvas, PADDING + (panelWidth + PADDING) * 1, PADDING + (panelHeight + PADDING) * 1, panelWidth, panelHeight, scale);
        panelRendering.paint    (canvas, PADDING + (panelWidth + PADDING) * 2, PADDING + (panelHeight + PADDING) * 1, panelWidth, panelHeight, scale);
        panelLegend.paint       (canvas, PADDING + (panelWidth + PADDING) * 3, PADDING + (panelHeight + PADDING) * 1, panelWidth, panelHeight * 2 + PADDING, scale);
        
        // Third row
        panelEvents.paint       (canvas, PADDING + (panelWidth + PADDING) * 0, PADDING + (panelHeight + PADDING) * 2, panelWidth * 2 + PADDING, panelHeight, scale);
        panelTheme.paint        (canvas, PADDING + (panelWidth + PADDING) * 2, PADDING + (panelHeight + PADDING) * 2, panelWidth, panelHeight, scale);

        // Colored bars
        try (var paint = new Paint()) {
            var barSize = 3 * scale;

            // left
            paint.setColor(0xFFe76f51);
            canvas.drawRect(Rect.makeXYWH(0, 0, barSize, 100 * scale), paint);
            canvas.drawRect(Rect.makeXYWH(0, height / 2 - 50 * scale, barSize, 100 * scale), paint);
            canvas.drawRect(Rect.makeXYWH(0, height - 100 * scale, barSize, 100 * scale), paint);

            // top
            paint.setColor(0xFF2a9d8f);
            canvas.drawRect(Rect.makeXYWH(0, 0, 100 * scale, barSize), paint);
            canvas.drawRect(Rect.makeXYWH(width / 2 - 50 * scale, 0, 100 * scale, barSize), paint);
            canvas.drawRect(Rect.makeXYWH(width - 100 * scale, 0, 100 * scale, barSize), paint);

            // right
            paint.setColor(0xFFe9c46a);
            canvas.drawRect(Rect.makeXYWH(width - barSize, 0, barSize, 100 * scale), paint);
            canvas.drawRect(Rect.makeXYWH(width - barSize, height / 2 - 50 * scale, barSize, 100 * scale), paint);
            canvas.drawRect(Rect.makeXYWH(width - barSize, height - 100 * scale, barSize, 100 * scale), paint);

            // bottom
            paint.setColor(0xFFFFFFFF);
            canvas.drawRect(Rect.makeXYWH(0, height - barSize, 100 * scale, barSize), paint);
            canvas.drawRect(Rect.makeXYWH(width / 2 - 50 * scale, height - barSize, 100 * scale, barSize), paint);
            canvas.drawRect(Rect.makeXYWH(width - 100 * scale, height - barSize, 100 * scale, barSize), paint);
        }
        canvas.restoreToCount(canvasCount);
    }

    @Override
    public void accept(Event e) {
        if (!initialized)
            return;

        if (e instanceof EventWindowClose) {
            if (App._windows.size() == 0)
                App.terminate();
            return;
        }
        
        panelTextInput.accept(e);
        panelScreens.accept(e);
        panelMouse.accept(e);
        panelMouseCursors.accept(e);
        panelRendering.accept(e);
        panelEvents.accept(e);
        panelTheme.accept(e);
        panelTouch.accept(e);

        float scale = window.getScreen().getScale();
        if (e instanceof EventKey eventKey) {
            if (eventKey.isPressed() == true && eventKey.isModifierDown(MODIFIER)) {
                switch (eventKey.getKey()) {
                    case P -> {
                        paused = !paused;
                        if (!paused)
                            window.requestFrame();
                    }
                    case N ->
                        new Example();
                    case H ->
                        window.setVisible(false);
                    case W ->
                        window.close();
                    case F ->
                        window.setFullScreen(!window.isFullScreen());
                    case O ->
                        window.setOpacity(window.getOpacity() == 1f ? 0.5f : 1f);
                    case UP ->
                        window.maximize();
                    case DOWN ->
                        window.restore();
                    case M ->
                        window.minimize();
                    case B ->
                        setProgressBar(progressBars.next());
                    case S -> {
                        var timer = new Timer();
                        // delay to allow workspace/focus switching
                        timer.schedule(new TimerTask() {
                            public void run() {
                                App.runOnUIThread(() -> {
                                        window.focus();
                                        timer.cancel();
                                });
                            }
                        }, 2000);
                    }
                }
            }
        } else if (e instanceof EventFrame) {
            if (!paused)
                window.requestFrame();
        } else if (e instanceof EventFrameSkija ee) {
            Surface s = ee.getSurface();
            paint(s.getCanvas(), s.getWidth(), s.getHeight());
        } else if (e instanceof EventWindowCloseRequest) {
            window.close();
        }
    }

    public void setProgressBar(String type) {
        progressBars.set(type);
        window.setProgressBar(switch (type) {
            case "0%" -> 0f;
            case "50%" -> 0.5f;
            case "100%" -> 1f;
            case "Indeterminate" -> 2f;
            default -> -1f;
        });
    }

    public static void main(String[] args) {
        App.start(() -> {
            new Example();
        });
    }
}
