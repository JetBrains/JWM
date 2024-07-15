package io.github.humbleui.jwm.examples;

import java.util.*;
import java.util.function.*;
import java.util.stream.*;

import io.github.humbleui.jwm.*;
import io.github.humbleui.skija.*;
import io.github.humbleui.types.*;

public class PanelMouseCursors extends Panel {
    public EventMouseMove lastMove = new EventMouseMove(0, 0, 0, 0, 0, 0);
    public Map<IRect, MouseCursor> rects = new HashMap<>();
    public boolean lastInside = false;
    public boolean keepCursor = false;
    public boolean cursorHidden = false;
    public boolean cursorLocked = false;

    public PanelMouseCursors(Window window) {
        super(window);
    }

    @Override
    public void accept(Event e) {
        if (e instanceof EventKey eventKey) {
            if (eventKey.isPressed() == true && eventKey.isModifierDown(Example.MODIFIER)) {
                switch (eventKey.getKey()) {
                    case Y -> {
                        window.hideMouseCursorUntilMoved(!cursorHidden);
                        cursorHidden = !cursorHidden;
                    }
                    case U -> {
                        window.lockMouseCursor(!cursorLocked);
                        cursorLocked = !cursorLocked;
                    }
                }
            }
        } else if (e instanceof EventMouseMove ee) {
            cursorHidden = false;
            lastMove = ee;
            var inside = contains(ee.getX(), ee.getY());
            if (inside || lastInside) {
                var relX = lastMove.getX() - lastX;
                var relY = lastMove.getY() - lastY;

                for (var rect: rects.keySet()) {
                    if (rect.contains(relX, relY)) {
                        var cursor = rects.get(rect);
                        if (window._lastCursor != cursor)
                            window.requestFrame();
                        window.setMouseCursor(cursor);
                        return;
                    }
                }
                if (!inside && lastInside) {
                    keepCursor = false;
                } else if (!keepCursor) {
                    if (window._lastCursor != MouseCursor.ARROW)
                       window.requestFrame();
                    window.setMouseCursor(MouseCursor.ARROW);
                }
                lastInside = inside;
            }
        } else if (e instanceof EventMouseButton ee && ee.isPressed() && lastInside) {
            keepCursor = true;
        }
    }

    @Override
    public void paintImpl(Canvas canvas, int width, int height, float scale) {
        try (var fg = new Paint().setColor(0x40FFFFFF);
             var hl = new Paint().setColor(0xFFFFFFFF);
             var bg = new Paint().setColor(0x40000000);)
        {
            var capHeight = (int) Example.FONT12.getMetrics().getCapHeight();
            var padding = (int) (8 * scale);
            var x = Example.PADDING;
            var y = Example.PADDING;
            rects.clear();
            for (var cursor: MouseCursor._values) {
                try (var line = TextLine.make(capitalize(cursor.toString()), Example.FONT12);) {
                    if (y + capHeight + 2 * padding > height - Example.PADDING) {
                        x += width / 2 - Example.PADDING / 2;
                        y = Example.PADDING;
                    }
                    var relX = lastMove.getX() - lastX;
                    var relY = lastMove.getY() - lastY;
                    var bounds = IRect.makeXYWH(x, y, (int) line.getWidth() + 2 * padding, capHeight + 2 * padding);
                    rects.put(bounds, cursor);
                    if (bounds.contains(relX, relY)) {
                        canvas.drawRRect(RRect.makeLTRB(bounds.getLeft(), bounds.getTop(), bounds.getRight(), bounds.getBottom(), 4 * scale), bg);
                        canvas.drawTextLine(line, x + padding, y + padding + capHeight, hl);
                    } else {
                        canvas.drawTextLine(line, x + padding, y + padding + capHeight, fg);
                    }
                    
                    y += capHeight + 2 * padding + 1;
                }
            }
        }
    }
}
