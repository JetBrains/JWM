#include "Buffer.hh"
#include <unistd.h>
#include <sys/mman.h>
#include <cerrno>
#include <time.h>
#include <fcntl.h>
using namespace jwm;

static void bufRelease(void* data, wl_buffer* wlbuffer) {
    auto buffer = reinterpret_cast<Buffer*>(data);
    delete buffer;
}
wl_buffer_listener Buffer::_bufferListener = {
    .release = bufRelease
};
static void randname(char *buf)
{
    	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}
static int _createShmFile() {
    int retries = 100;
    do {
        char name[] = "/wl_shm-XXXXXX";
        randname(name + sizeof(name) - 7);
        --retries;
        int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd >= 0) {
            shm_unlink(name);
            return fd;
        }
    } while (retries > 0 && errno == EEXIST);
    return -1;
}

static int _allocateShmFile(size_t size) {
    int fd = _createShmFile();
    if (fd < 0)
        return -1;
    int ret;
    do {
        ret = ftruncate(fd, size);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}
Buffer::Buffer(wl_buffer* buffer,
                int width,
                int height,
                void *data,
                size_t dataSize) :
    _buffer(buffer),
    _width(width),
    _height(height),
    _data(data),
    _dataSize(dataSize) 
{
    wl_buffer_add_listener(buffer, &_bufferListener, this);
}
Buffer::~Buffer()
{
    wl_buffer_destroy(_buffer);
    munmap(_data, _dataSize);
}

Buffer* Buffer::createShmBuffer(wl_shm* shm, int width, int height, uint32_t format) 
{
    wl_shm_pool* pool;
    int fd, size, stride;
    void* data;
    wl_buffer* buffer;

    stride = width * 4;
    size = stride * height;
    
    fd = _allocateShmFile(size);
    if (fd < 0)
        return nullptr;

    data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return nullptr;
    }

    pool = wl_shm_create_pool(shm, fd, size);

    buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);

    wl_shm_pool_destroy(pool);
    close(fd);

    return new Buffer(buffer, width, height, data, size);
}
