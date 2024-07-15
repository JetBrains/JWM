package io.github.humbleui.jwm.skija;

import io.github.humbleui.jwm.*;
import io.github.humbleui.skija.*;
import lombok.*;
import org.jetbrains.annotations.*;

public class LayerGLSkija extends LayerGL {
    @Getter @Setter @ApiStatus.Internal public int _samples = 0;
    @Getter @Setter @ApiStatus.Internal public int _stencil = 8;
    @Getter @Setter @ApiStatus.Internal public int _fbId = 0;
    @Getter @Setter @ApiStatus.Internal public int _fbFormat = FramebufferFormat.GR_GL_RGBA8;
    @Getter @Setter @ApiStatus.Internal public SurfaceOrigin _origin = SurfaceOrigin.BOTTOM_LEFT;
    @Getter @Setter @ApiStatus.Internal public SurfaceColorFormat _colorFormat = SurfaceColorFormat.RGBA_8888;
    @Getter @Setter @ApiStatus.Internal public ColorSpace _colorSpace = ColorSpace.getSRGB(); // TODO load monitor profile
    @Getter @Setter @ApiStatus.Internal public SurfaceProps _surfaceProps = new SurfaceProps(PixelGeometry.RGB_H);

    @Getter @ApiStatus.Internal public DirectContext _directContext = null;
    @Getter @ApiStatus.Internal public BackendRenderTarget _renderTarget = null;
    @Getter @ApiStatus.Internal public Surface _surface = null;

    @Override
    public void frame() {
        makeCurrent();
        
        if (_directContext == null)
            _directContext = DirectContext.makeGL();

        if (_renderTarget == null)
            _renderTarget = BackendRenderTarget.makeGL(_width, _height, _samples, _stencil, _fbId, _fbFormat);

        if (_surface == null)
            _surface = Surface.makeFromBackendRenderTarget(_directContext, _renderTarget, _origin, _colorFormat, _colorSpace, _surfaceProps);

        _window.accept(new EventFrameSkija(_surface));

        _surface.flushAndSubmit();
        swapBuffers();
    }

    @Override
    public void resize(int width, int height) {
        if (_surface != null) {
            _surface.close();
            _surface = null;
        }

        if (_renderTarget != null) {
            _renderTarget.close();
            _renderTarget = null;
        }

        if (_directContext != null) {
            // https://bugs.chromium.org/p/skia/issues/detail?id=12814
            // _directContext.abandon();
            _directContext.close();
            _directContext = null;
        }
        
        super.resize(width, height);
    }

    @Override
    public void close() {
        assert !isClosed() : "Layer is already closed";
        if (_directContext != null) {
            // https://bugs.chromium.org/p/skia/issues/detail?id=12814
            // _directContext.abandon();
            _directContext.close();
            _directContext = null;
        }

        if (_surface != null) {
            _surface.close();
            _surface = null;
        }

        if (_renderTarget != null) {
            _renderTarget.close();
            _renderTarget = null;
        }

        super.close();
    }
}
