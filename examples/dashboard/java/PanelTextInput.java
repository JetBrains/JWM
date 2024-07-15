package io.github.humbleui.jwm.examples;

import java.util.*;
import java.util.function.*;
import io.github.humbleui.jwm.*;
import io.github.humbleui.skija.*;
import io.github.humbleui.types.*;

public class PanelTextInput extends Panel implements TextInputClient {
    public List<String> keys = Collections.synchronizedList(new ArrayList<String>());
    
    public String text = "";
    public EventTextInputMarked lastMarked = null;
    public int lastInputHeight = 0;
    public boolean cursorDraw = true;
    public TimerTask timerTask;
    public static Timer timer = new Timer(true);
    public boolean _wasInside = false;

    public PanelTextInput(Window window) {
        super(window);
        this.drawBG = false;
        window.setTextInputEnabled(true);
        window.setTextInputClient(this);
        timerTask = new TimerTask() {
            public void run() {
                cursorDraw = !cursorDraw;
                App.runOnUIThread(() -> { if (!window.isClosed()) window.requestFrame(); });
            }
        };
        timer.schedule(timerTask, 0, 500);
    }

    @Override
    public void accept(Event e) {
        if (e instanceof EventTextInput ee) {
            int start = ee.getReplacementStart() == -1 ? text.length() : ee.getReplacementStart();
            int end = start + ee.getReplacementEnd() - ee.getReplacementStart();
            text = text.substring(0, start) + ee.getText() + text.substring(end);
            lastMarked = null;
            window.requestFrame();
        } else if (e instanceof EventTextInputMarked ee) {
            int start = ee.getReplacementStart() == -1 ? text.length() : ee.getReplacementStart();
            int end = start + ee.getReplacementEnd() - ee.getReplacementStart();
            text = text.substring(0, start) + text.substring(end);
            lastMarked = ee;
            window.requestFrame();
        } else if (e instanceof EventMouseButton ee) {
            window.unmarkText();
            window.requestFrame();
        } else if (e instanceof EventMouseMove ee) {
            boolean isInside = contains(ee.getX(), ee.getY());
            if (!_wasInside && isInside)
                window.setMouseCursor(MouseCursor.IBEAM);
            if (_wasInside && !isInside)
                window.setMouseCursor(MouseCursor.ARROW);
            _wasInside = isInside;
        } else if (e instanceof EventKey ee) {
            Key key = ee.getKey();
            KeyLocation loc = ee.getLocation();

            var keyText = loc == KeyLocation.DEFAULT ? key.getName() : capitalize(loc.toString()) + " " + key.getName();
            if (ee.isPressed() && !keys.contains(keyText))
                keys.add(keyText);
            else if (!ee.isPressed())
                keys.remove(keyText);

            if (ee.isPressed()) {
                switch (key) {
                    case ENTER -> text += "\n";

                    case BACKSPACE -> {
                        if (lastMarked == null && text.length() > 0) {
                            try (var iter = BreakIterator.makeCharacterInstance();) {
                                iter.setText(text);
                                text = text.substring(0, iter.preceding(text.length()));
                            }
                        }
                    }
                    
                    case SPACE -> {
                        if (Platform.CURRENT == Platform.MACOS
                            && ee.isModifierDown(KeyModifier.CONTROL)
                            && ee.isModifierDown(KeyModifier.MAC_COMMAND))
                        {
                            App.openSymbolsPalette();
                        }
                    }
                }

                if (ee.isModifierDown(Example.MODIFIER)) {
                    switch (key) {
                        case C ->
                            Clipboard.set(ClipboardEntry.makePlainText(text));
                        case V -> {
                            window.unmarkText();
                            ClipboardEntry entry = Clipboard.get(ClipboardFormat.TEXT);
                            if (entry != null)
                                text += entry.getString();
                        }
                        case X -> {
                            ClipboardFormat[] formats = Clipboard.getFormats();
                            if (formats != null)
                                for (ClipboardFormat format: formats)
                                    System.out.println(format.getFormatId());
                        }
                    }
                }
            }
            window.requestFrame();
        }
    }

    @Override
    public void paintImpl(Canvas canvas, int width, int height, float scale) {
        var capHeight = Example.FONT12.getMetrics().getCapHeight();
        var padding = (int) 8 * scale;
        lastInputHeight = (int) (height - capHeight - 3 * padding);

        // keys
        try (var bg = new Paint().setColor(0x40000000);
             var fg = new Paint().setColor(0xFFFFFFFF); )
        {
            var x = 0;
            var y = lastInputHeight + padding;
            for (var key: keys) {
                try (var line = TextLine.make(key, Example.FONT12); ) {
                    canvas.drawRRect(RRect.makeXYWH(x, y, line.getWidth() + 2 * padding, capHeight + 2 * padding, 4 * scale), bg);
                    canvas.drawTextLine(line, x + padding, y + capHeight + padding, fg);
                    x += line.getWidth() + 3 * padding;
                }
            }
        }

        // text input
        try (var paint = new Paint(); ) {
            canvas.save();
            paint.setColor(0xFFFFFFFF);
            canvas.drawRRect(RRect.makeXYWH(0, 0, width, lastInputHeight, 4 * scale), paint);

            Font font = Example.FONT24;
            FontMetrics metrics = font.getMetrics();

            var y = lastInputHeight - Example.PADDING - metrics.getDescent();
            var lines = text.split("\n", -1);
            var i = lines.length - 1;

            // Last line
            try (var line = TextLine.make(lines[i], font)) {
                canvas.save();
                canvas.translate(Example.PADDING, y);
                paint.setColor(0xFF000000);
                canvas.drawTextLine(line, 0, 0, paint);
                canvas.translate(line.getWidth(), 0);

                // marked text
                paint.setColor(0xFF0087D8);
                if (lastMarked != null) {

                    try (var marked = TextLine.make(lastMarked.getText(), font)) {
                        canvas.drawTextLine(marked, 0, 0, paint);
                        var start = marked.getCoordAtOffset(lastMarked.getSelectionStart());
                        var end = marked.getCoordAtOffset(lastMarked.getSelectionEnd());
                        if (0 < start - 1 * scale)
                            canvas.drawRect(Rect.makeLTRB(0, 2 * scale, start - 1 * scale, 4 * scale), paint);
                        if (start + 1 * scale < end - 1 * scale)
                            canvas.drawRect(Rect.makeLTRB(start + 1 * scale, 2 * scale, end - 1 * scale, 6 * scale), paint);
                        if (end + 1 * scale < marked.getWidth())
                            canvas.drawRect(Rect.makeLTRB(end + 1 * scale, 2 * scale, marked.getWidth(), 4 * scale), paint);

                        canvas.translate(marked.getWidth(), 0);
                    }
                }

                // cursor (cursorDraw used for blink animation)
                if (cursorDraw)
                    canvas.drawRect(Rect.makeXYWH(0, metrics.getAscent(), 2 * scale, metrics.getHeight()), paint);

                canvas.restore();
            }

            paint.setColor(0xFF000000);
            for (i = lines.length - 2; i >= 0; i--) {
                y -= metrics.getHeight();
                try (var line = TextLine.make(lines[i], font)) {
                    canvas.drawTextLine(line, Example.PADDING, y, paint);
                }
            }
            canvas.restore();
        }
    }

    @Override
    public IRect getRectForMarkedRange(int selectionStart, int selectionEnd) {
        Font font = Example.FONT24;
        FontMetrics metrics = font.getMetrics();

        var lines = text.split("\n", -1);
        try (var line = TextLine.make(lines[lines.length - 1], font);) {
            var left = lastX + Example.PADDING + line.getWidth();
            var top = lastY + lastInputHeight - Example.PADDING - metrics.getHeight();

            if (lastMarked != null) {
                try (var marked = TextLine.make(lastMarked.getText(), font)) {
                    var start = marked.getCoordAtOffset(selectionStart);
                    var end = marked.getCoordAtOffset(selectionEnd);
                    return IRect.makeXYWH((int) (left + start),
                                           (int) top,
                                           (int) (end - start),
                                           (int) metrics.getHeight());
                }
            } else
                return IRect.makeXYWH((int) left, (int) top, 0, (int) metrics.getHeight());
        }
    }

    @Override
    public IRange getSelectedRange() {
        int len = text.length();
        return new IRange(len, len);
    }

    @Override
    public IRange getMarkedRange() {
        return new IRange(-1, -1);
    }

    @Override
    public String getSubstring(int start, int end) {
        int len = text.length();
        int start2 = Math.min(start, len);
        int end2 = Math.min(end, start2);
        return text.substring(start2, end2);
    }
}
