#pragma once

#include <wayland-client.h>
#include <vector>
#include <cstdint>

namespace jwm {
    class Buffer {
    public:
        Buffer(wl_buffer* buffer,
                int width,
                int height,
                void *data,
                size_t dataSize);
        ~Buffer();
        wl_buffer* _buffer = nullptr;
        wl_buffer* getBuffer() const {
            return _buffer;
        }
        int _width;
        int _height;
        void* _data;
        void* getData() const {
            return _data;
        }
        size_t _dataSize;
        size_t getSize() {
            return _dataSize;
        }

        static wl_buffer_listener _bufferListener;

        static Buffer* createShmBuffer(wl_shm* shm, int width, int height, uint32_t format);
    private:
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) = delete;
        
    };
}
