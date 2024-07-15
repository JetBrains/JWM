package io.github.humbleui.jwm.examples;

import java.util.*;
import java.util.concurrent.*;
import java.util.function.*;
import java.util.stream.*;

import io.github.humbleui.jwm.*;
import io.github.humbleui.skija.*;
import io.github.humbleui.types.*;

public class PanelRendering extends Panel {
    public boolean vsyncColor = false;

    public long t0 = System.nanoTime();
    public double[] times = new double[180];
    public int timesIdx = 0;

    public Map<String, String> layersStatus = new HashMap<>();
    public String[] layers;
    public int layerIdx = 0;

    // Layer status displayed on the right side from the layer name
    public static final String CHECKED = "+";
    public static final String FAILED = "x";
    public static final String UNKNOWN = "?";

    public PanelRendering(Window window) {
        super(window);

        if (Platform.CURRENT == Platform.MACOS)
            layers = new String[] { "LayerMetalSkija", "LayerGLSkija" };
        else if (Platform.CURRENT == Platform.WINDOWS)
            layers = new String[] { "LayerD3D12Skija", "LayerGLSkija", "SkijaLayerRaster" };
        else if (Platform.CURRENT == Platform.X11)
            layers = new String[] { "LayerGLSkija", "LayerRasterSkija" };
        else if (Platform.CURRENT == Platform.WAYLAND)
            layers = new String[] { "LayerGLSkija", "LayerRasterSkija" };

        for (var layerName: layers)
            layersStatus.put(layerName, UNKNOWN);

        changeLayer();
    }

    public void changeLayer() {
        int attemptsCount = layers.length;

        while (attemptsCount > 0) {
            attemptsCount -= 1;
            String layerName = layers[layerIdx];
            String className = "io.github.humbleui.jwm.skija." + layerName;

            try {
                Layer layer = (Layer) Example.class.forName(className).getDeclaredConstructor().newInstance();
                window.setLayer(layer);
                break;
            } catch (Exception e) {
                System.err.println("Failed to create layer " + className);
                e.printStackTrace();
                layersStatus.put(layerName, FAILED); // Update layer status
                nextLayerIdx();
            }
        }

        if (window._layer == null)
            throw new RuntimeException("No available layer to create");

        layersStatus.put(layers[layerIdx], CHECKED); // Mark layer as checked
    }

    @Override
    public void accept(Event e) {
        if (e instanceof EventKey ee && ee.isPressed()) {
            Key key = ee.getKey();
            boolean modifier = ee.isModifierDown(Example.MODIFIER);
            if (Key.L == key && modifier) {
                nextLayerIdx();
                changeLayer();
            }
        }
    }

    @Override
    public void paintImpl(Canvas canvas, int width, int height, float scale) {
        try (var paint = new Paint();) {
            paint.setColor(0xFFFFFFFF);
            var metrics = Example.FONT12.getMetrics();

            // FPS graph
            canvas.save();
            canvas.translate(Example.PADDING, height - Example.PADDING - 32 * scale);

            paint.setColor(0x4033cc33);
            canvas.drawRRect(RRect.makeXYWH(0, 0, width - Example.PADDING * 2, 32 * scale, 4 * scale, 4 * scale, 0, 0), paint);
            paint.setColor(0xFF33CC33);
            for (int i = 0; i < times.length; ++i) {
                var idx = (timesIdx + i) % times.length;
                canvas.drawRect(Rect.makeXYWH(i * scale,
                                              Math.min(height, (32 - (float) times[idx]) * scale),
                                              1 * scale,
                                              (float) (times[idx] * scale)),
                                paint);
            }

            paint.setColor(0x20000000);
            canvas.drawRect(Rect.makeXYWH(0, (32 - 17) * scale, times.length * scale, 1 * scale), paint);
            canvas.drawRect(Rect.makeXYWH(0, (32 - 8) * scale, times.length * scale, 1 * scale), paint);
            canvas.restore();

            canvas.save();
            paint.setColor(0xFFFFFFFF);

            // Layers (also paint layer status on the right side)
            try (var shevron = TextLine.make("> ", Example.FONT12);) {
                
                canvas.translate(Example.PADDING, Example.PADDING - metrics.getAscent());
                for (int i = 0; i < layers.length; ++i) {
                    if (i == layerIdx)
                        canvas.drawTextLine(shevron, 0, 0, paint);

                    String status = layersStatus.get(layers[i]);
                    String displayString = layers[i] + " " + layersStatus.get(layers[i]);

                    switch (status) {
                        case FAILED ->
                            paint.setColor(0xFFFF5252);
                        case UNKNOWN ->
                            paint.setColor(0xFFFEC942);
                        default ->
                            paint.setColor(0xFF74DD1B);
                    }

                    canvas.drawString(displayString, shevron.getWidth(), 0, Example.FONT12, paint);
                    canvas.translate(0, metrics.getHeight());
                }
            }

            paint.setColor(0xFFFFFFFF);

            // Paint counters
            int len = (int) ((width - Example.PADDING * 2) / scale);
            if (len > 0 && times.length != len) {
                times = new double[len];
                timesIdx = 0;
            }

            long t1 = System.nanoTime();
            times[timesIdx] = (t1 - t0) / 1000000.0;
            t0 = t1;
            timesIdx = (timesIdx + 1) % times.length;
            int frames = 0;
            double time = 0;
            for (int i = 0; i < times.length; ++i) {
                var idx = (timesIdx - i + times.length) % times.length;
                if (times[idx] > 0) {
                    time += times[idx];
                    frames++;
                }
                if (time > 1000)
                    break;
            }
            String fps = String.format("%.01f", (frames / time * 1000));
            canvas.drawString("FPS: " + fps, 0, 0, Example.FONT12, paint);

            // VSync
            try (var line = TextLine.make("VSYNC", Example.FONT24); ) {
                var capHeight = Example.FONT24.getMetrics().getCapHeight();
                paint.setColor(0xFFE0E0E0);
                canvas.drawRRect(RRect.makeXYWH(width - line.getWidth() - 3 * Example.PADDING,
                                                Example.PADDING,
                                                line.getWidth() + 2 * Example.PADDING,
                                                capHeight + 2 * Example.PADDING,
                                                4 * scale), paint);
                
                paint.setColor(vsyncColor ? 0xFFEF8784 : 0xFFA1FCFE);
                canvas.drawTextLine(line, width - line.getWidth() - 2 * Example.PADDING, capHeight + 2 * Example.PADDING, paint);
                vsyncColor = !vsyncColor;
            }

            canvas.restore();
        }
    }

    private int nextLayerIdx() {
        layerIdx = (layerIdx + 1) % layers.length;
        return layerIdx;
    }
}
