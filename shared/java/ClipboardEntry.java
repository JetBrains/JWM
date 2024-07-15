package io.github.humbleui.jwm;

import org.jetbrains.annotations.*;
import lombok.*;

@Data
public class ClipboardEntry {
    @ApiStatus.Internal public final ClipboardFormat _format;
    @ApiStatus.Internal public final byte[] _data;

    /**
     * <p>Make new clipboard entry from format and raw byte data.</p>
     *
     * @param format        clipboard entry format
     * @param data          raw byte data
     * @return              clipboard entry
     */
    @NotNull
    public static ClipboardEntry make(ClipboardFormat format, byte[] data) {
        return new ClipboardEntry(format, data);
    }

    /**
     * <p>Make new plain text entry from plain text.</p>
     *
     * @param text          raw string text
     * @return              clipboard entry
     */
    @NotNull @SneakyThrows
    public static ClipboardEntry makePlainText(String text) {
        return makeString(ClipboardFormat.TEXT, text);
    }

    /**
     * <p>Make new html entry from plain text.</p>
     *
     * @param text          raw string text
     * @return              clipboard entry
     */
    @NotNull @SneakyThrows
    public static ClipboardEntry makeHTML(String text) {
        return makeString(ClipboardFormat.HTML, text);
    }

    /**
     * <p>Make new rtf entry from plain text.</p>
     *
     * @param text          raw string text
     * @return              clipboard entry
     */
    @NotNull @SneakyThrows
    public static ClipboardEntry makeRTF(String text) {
        return makeString(ClipboardFormat.RTF, text);
    }

    /**
     * <p>Make new clipboard entry from format and plain text.</p>
     *
     * @param format        clipboard entry format
     * @param text          raw string text
     * @return              clipboard entry
     */
    @NotNull @SneakyThrows
    public static ClipboardEntry makeString(ClipboardFormat format, String text) {
        if (Platform.CURRENT == Platform.X11 || Platform.CURRENT == Platform.MACOS || Platform.CURRENT == Platform.WAYLAND) {
            return make(format, text.getBytes("UTF-8"));
        }
        return make(format, text.getBytes("UTF-16LE"));
    }

    /**
     * <p>Get clipboard entry content as string text.</p>
     * <p>If data cannot be converted to the string, this function throws exception.</p>
     *
     * @return              clipboard data converted to the string
     */
    @NotNull @SneakyThrows
    public String getString() {
        if (Platform.CURRENT == Platform.X11 || Platform.CURRENT == Platform.MACOS || Platform.CURRENT == Platform.WAYLAND) {
            return new String(_data, "UTF-8");
        }
        return new String(_data, "UTF-16LE");
    }
}
