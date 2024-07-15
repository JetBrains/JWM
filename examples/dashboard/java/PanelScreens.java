package io.github.humbleui.jwm.examples;

import java.util.function.*;
import io.github.humbleui.jwm.*;
import io.github.humbleui.skija.*;
import io.github.humbleui.types.*;

public class PanelScreens extends Panel {
    public EventWindowResize lastResize = new EventWindowResize(0, 0, 0, 0);
    public EventWindowMove lastMove = new EventWindowMove(0, 0);
    public Paint stroke = new Paint().setMode(PaintMode.STROKE).setColor(0x80FFFFFF);
    public Paint fill = new Paint().setColor(0x20FFFFFF);
    public Paint white = new Paint().setColor(0xFFFFFFFF);
    public int idx = 0;
    public Options titleStyles = new Options("Default", "Hidden");

    public PanelScreens(Window window) {
        super(window);
        if (Platform.MACOS == Platform.CURRENT) {
            titleStyles = new Options("Default", "Hidden", "Transparent", "Unified", "Unified Compact", "Unified Transparent", "Unified Compact Transparent");
        } else if (Platform.X11 == Platform.CURRENT || Platform.WAYLAND == Platform.CURRENT) {
            titleStyles = new Options("Default", "Hidden");
        }
    }

    public void setTitleStyle(String style) {
        titleStyles.set(style);

        if (Platform.MACOS == Platform.CURRENT) {
            WindowMac w = (WindowMac) window;
            switch (style) {
                case "Default" -> {
                    w.setTitlebarStyle(WindowMacTitlebarStyle.DEFAULT);
                    w.setFullSizeContentView(false);
                }
                case "Hidden" -> {
                    window.setTitlebarVisible(false);
                }
                case "Transparent" -> {
                    window.setTitlebarVisible(true);
                    w.setTitlebarStyle(WindowMacTitlebarStyle.DEFAULT);
                    w.setFullSizeContentView(true);
                    w.setTrafficLightPosition(0, 0);
                }
                case "Unified" -> {
                    w.setTitlebarStyle(WindowMacTitlebarStyle.UNIFIED);
                    w.setFullSizeContentView(false);
                }
                case "Unified Compact" -> {
                    w.setTitlebarStyle(WindowMacTitlebarStyle.UNIFIED_COMPACT);
                }
                case "Unified Transparent" -> {
                    w.setTitlebarStyle(WindowMacTitlebarStyle.UNIFIED);
                    w.setFullSizeContentView(true);
                }
                case "Unified Compact Transparent" -> {
                    w.setTitlebarStyle(WindowMacTitlebarStyle.UNIFIED_COMPACT);
                    w.setFullSizeContentView(true);
                }
            }
        } else if (Platform.X11 == Platform.CURRENT) {
            WindowX11 w = (WindowX11) window;
            switch (style) {
            case "Default" ->
                w.setTitlebarVisible(true);
            case "Hidden" ->
                w.setTitlebarVisible(false);
            }
        } else if (Platform.WAYLAND == Platform.CURRENT) {
            WindowWayland w = (WindowWayland) window;
            switch (style) {
            case "Default" ->
                w.setTitlebarVisible(true);
            case "Hidden" ->
                w.setTitlebarVisible(false);
            }
        }
    }

    @Override
    public void accept(Event e) {
        float scale = window.getScreen().getScale();
        if (e instanceof EventKey eventKey) {
            if (eventKey.isPressed() == true && eventKey.isModifierDown(Example.MODIFIER)) {
                switch (eventKey.getKey()) {
                    case DIGIT1 ->
                        window.minimize();
                    case DIGIT2 ->
                        window.maximize();
                    case DIGIT3 ->
                        window.restore();
                    case DIGIT4 -> {
                        window.setVisible(false);
                    }
                    case DIGIT5 -> {
                        Screen[] screens = App.getScreens();
                        idx = (idx + 1) % (screens.length * 5);
                        IRect bounds = screens[idx / 5].getWorkArea();
                        switch (idx % 5) {
                            case 0 -> window.setWindowPosition(bounds.getLeft() + bounds.getWidth() / 4, bounds.getTop() + bounds.getHeight() / 4);
                            case 1 -> window.setWindowPosition(bounds.getLeft(), bounds.getTop());
                            case 2 -> window.setWindowPosition(bounds.getLeft() + bounds.getWidth() / 2, bounds.getTop());
                            case 3 -> window.setWindowPosition(bounds.getLeft(), bounds.getTop() + bounds.getHeight() / 2);
                            case 4 -> window.setWindowPosition(bounds.getLeft() + bounds.getWidth() / 2, bounds.getTop() + bounds.getHeight() / 2);
                        }
                    }
                    case DIGIT6 -> {
                        IRect bounds = window.getScreen().getWorkArea();
                        int width  = (int) (((int) ((bounds.getWidth() / 2) / scale)) * scale);
                        int height = (int) (((int) ((bounds.getHeight() / 2) / scale)) * scale);
                        if (window.getWindowRect().getWidth() != width || window.getWindowRect().getHeight() != height) {
                            window.setWindowSize(width, height);
                        } else {
                            window.setContentSize(width, height);
                        }
                    }
                    case T -> {
                        setTitleStyle(titleStyles.next());
                        window.focus();
                    }
                }
            }
        } if (e instanceof EventWindowResize ee) {
            lastResize = ee;
            window.requestFrame();
        } else if (e instanceof EventWindowMove ee) {
            lastMove = ee;
            window.requestFrame();
        }
    }

    public void drawRect(Canvas canvas, IRect rect) {
        canvas.drawRect(Rect.makeXYWH(rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight()), fill);
        canvas.drawRect(Rect.makeXYWH(rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight()), stroke);
    }

    @Override
    public void paintImpl(Canvas canvas, int width, int height, float scale) {
        float minX = 0, minY = 0, maxX = 0, maxY = 0;
        for (var screen: App.getScreens()) {
            var bounds = screen.getBounds();
            minX = Math.min(minX, bounds.getLeft());
            minY = Math.min(minY, bounds.getTop());
            maxX = Math.max(maxX, bounds.getRight());
            maxY = Math.max(maxY, bounds.getBottom());
        }

        canvas.save();
        float scale2 = Math.min((width - Example.PADDING * 2) / (maxX - minX),
                                (height - Example.PADDING * 2) / (maxY - minY));
        canvas.translate(Example.PADDING, Example.PADDING);
        canvas.scale(scale2, scale2);
        canvas.translate(-minX, (height - Example.PADDING * 2) / scale2 - maxY);
        stroke.setStrokeWidth(1 * scale / scale2);
        for (var screen: App.getScreens()) {
            stroke.setColor(screen.isPrimary() ? 0x80CC3333 : 0x80FFFFFF);
            drawRect(canvas, screen.getBounds());
            drawRect(canvas, screen.getWorkArea());
        }
        IRect windowRect = window.getWindowRect();
        drawRect(canvas, windowRect);
        drawRect(canvas, window.getContentRectAbsolute());
        stroke.setColor(0x80CC3333);
        drawRect(canvas, IRect.makeXYWH(lastMove.getWindowLeft(), lastMove.getWindowTop(), lastResize.getWindowWidth(), lastResize.getWindowHeight()));
        canvas.restore();

        var contentRect = window.getContentRect();
        var capHeight = Example.FONT12.getMetrics().getCapHeight();
        var lineHeight = capHeight + (int) 8 * scale;
        canvas.save();
        canvas.translate(Example.PADDING, Example.PADDING + capHeight);
        canvas.drawString("Position: " + windowRect.getLeft() + ", " + windowRect.getTop(), 0, 0, Example.FONT12, white);
        canvas.translate(0, lineHeight);
        canvas.drawString("Window size: " + windowRect.getWidth() + ", " + windowRect.getHeight(), 0, 0, Example.FONT12, white);
        canvas.translate(0, lineHeight);
        canvas.drawString("Content size: " + contentRect.getWidth() + ", " + contentRect.getHeight(), 0, 0, Example.FONT12, white);
        canvas.translate(0, lineHeight);
        canvas.drawString("Titlebar: " + titleStyles.get(), 0, 0, Example.FONT12, white);
        canvas.translate(0, lineHeight);
        canvas.restore();
    }
}
