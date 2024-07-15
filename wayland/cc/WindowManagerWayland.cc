#include "WindowManagerWayland.hh"
#include "WindowWayland.hh"
#include <cerrno>
#include <cstdio>
#include <limits>
#include <impl/Library.hh>
#include <impl/JNILocal.hh>
#include "AppWayland.hh"
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include "KeyWayland.hh"
#include "MouseButtonWayland.hh"
#include "StringUTF16.hh"
#include <algorithm>
#include <system_error>
#include "Log.hh"
#include <cstring>
#include "Output.hh"
#include <linux/input-event-codes.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>
#include <algorithm>
#include <chrono>

using namespace jwm;

wl_registry_listener WindowManagerWayland::_registryListener = {
    .global = WindowManagerWayland::registryHandleGlobal,
    .global_remove = WindowManagerWayland::registryHandleGlobalRemove
};
wl_seat_listener WindowManagerWayland::_seatListener = {
    .capabilities = WindowManagerWayland::seatCapabilities,
    .name = WindowManagerWayland::seatName
};

static void xdgWmBasePing(void* data, xdg_wm_base* wm, uint32_t serial) {
    xdg_wm_base_pong(wm, serial);    
}
static xdg_wm_base_listener _wmListener = {
    .ping = xdgWmBasePing
};
WindowManagerWayland::WindowManagerWayland():
    display(wl_display_connect(nullptr)) {
        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &_registryListener, this);
        wl_display_roundtrip(display);
        


        if (!(shm && compositor && deviceManager && seat && xdgWm && subcompositor && viewporter)) {
            // ???
            // Bad. Means our compositor no supportie : (
            throw std::system_error(ENOTSUP, std::generic_category(), "Unsupported compositor");
        }
        
        xdg_wm_base_add_listener(xdgWm, &_wmListener, nullptr);
        // ???: Moving this after libdecor_new causes input to not work
        wl_seat_add_listener(seat, &_seatListener, this);
        dataDevice = wl_data_device_manager_get_data_device(deviceManager, seat);
        wl_data_device_add_listener(dataDevice, &_deviceListener, this);


        wl_display_roundtrip(display);

}




void WindowManagerWayland::runLoop() {
    _runLoop = true;
    int pipes[2];

    char buf[100];
    if (pipe(pipes)) {
        printf("failed to open pipe\n");
        return;
    }
    notifyFD = pipes[1];
    fcntl(pipes[1], F_SETFL, O_NONBLOCK); // notifyLoop no blockie : )
    struct pollfd wayland_out = {.fd=wl_display_get_fd(display),.events=POLLOUT};
    struct pollfd ps[] = {
        {.fd=wl_display_get_fd(display), .events=POLLIN}, 
        {.fd=pipes[0], .events=POLLIN},
    };

    // who be out here running they loop
    while (_runLoop) {
        if (jwm::classes::Throwable::exceptionThrown(app.getJniEnv()))
            _runLoop = false;
        while (wl_display_prepare_read(display) != 0) {
            wl_display_dispatch_pending(display);

        }
        while (true) {
            int res = wl_display_flush(display);
            if (res >= 0)
                break;
            
            switch (errno) {
                case EPIPE:
                    wl_display_read_events(display);
                    throw std::system_error(errno, std::generic_category(), "connection to wayland server unexpectedly terminated");
                    break;
                case EAGAIN:
                    if (poll(&wayland_out, 1, -1) < 0) {
                        throw std::system_error(EPIPE, std::generic_category(), "poll failed");
                    }
                    break;
                default: 
                    throw std::system_error(errno, std::generic_category(), "failed to flush requests");
                    break;
            }

        }
        // block until event : )
        int timeout = -1;
        if (_keyboard && _keyboard->_repeating && _keyboard->getFocus()) {
            if (_keyboard->_repeatRate > 0) {
                auto now = std::chrono::steady_clock::now();
                auto target = _keyboard->_nextRepeat;
                if (now < target) {
                    timeout = std::chrono::duration_cast<std::chrono::milliseconds>(target - now).count();
                } else {
                    _processKeyboard();
                }
            }
        }
        if (poll(&ps[0], 2, timeout) < 0) {
            printf("error with pipe\n");
            wl_display_cancel_read(display);
            break;
        }
        if (ps[0].revents & POLLIN) {
            if (wl_display_read_events(display) < 0) {
                std::perror("events failed");
                break;
            }
        } else {
            wl_display_cancel_read(display);
        }

        if (ps[1].revents & POLLIN) {
            while (read(pipes[0], buf, sizeof(buf)) == sizeof(buf)) { }
        }
        if (ps[0].revents & POLLIN || ps[1].revents & POLLIN) {
            _processCallbacks();
        } else {
            // don't test if we already calculated earlier
            _processKeyboard();
        }
        
        wl_display_dispatch_pending(display);
        notifyBool.store(false);
    }

    notifyFD = -1;
    close(pipes[0]);
    close(pipes[1]);
    
}

void WindowManagerWayland::_processCallbacks() {
    {
        // process ui thread callbacks
        std::unique_lock<std::mutex> lock(_taskQueueLock);
        while (!_taskQueue.empty()) {
            auto callback = std::move(_taskQueue.front());
            _taskQueue.pop();
            lock.unlock();
            callback();
            lock.lock();
        }
    }
    {
        std::list<WindowWayland*> copy(_windows);
        // process redraw requests
        for (auto p : copy) {
            if (p->isRedrawRequested()) {
                p->unsetRedrawRequest();
                if (p->_visible && p->isConfigured()) {
                    if (p->_layer) {
                        p->_layer->makeCurrent();
                    }
                    p->dispatch(classes::EventFrame::kInstance);
                }
            }
        }
    }
   
}
void WindowManagerWayland::_processKeyboard() {
    if (!_keyboard || !_keyboard->_repeating) return;
    auto focus = _keyboard->getFocus();
    if (!focus) return;
    auto now = std::chrono::steady_clock::now();
    _keyboard->_nextRepeat = now + std::chrono::milliseconds(_keyboard->_repeatRate);
    auto env = jwm::app.getJniEnv();
    jwm::KeyLocation location = jwm::KeyLocation::DEFAULT;
    JNILocal<jobject> keyOffEvent(
        env,
        classes::EventKey::make(
                env,
                _keyboard->_repeatKey,
                false,
                KeyWayland::getModifiers(_keyboard->_state),
                location
            )
        );

    JNILocal<jobject> keyEvent(
        env,
        classes::EventKey::make(
                env,
                _keyboard->_repeatKey,
                true,
                KeyWayland::getModifiers(_keyboard->_state),
                location
            )
        );
    focus->dispatch(keyOffEvent.get());
    focus->dispatch(keyEvent.get());
    if (_keyboard->_repeatingText) {
        jwm::JNILocal<jstring> jtext = _keyboard->_repeatText.toJString(env);
        jwm::JNILocal<jobject> eventTextInput(env, classes::EventTextInput::make(env, jtext.get()));

        focus->dispatch(eventTextInput.get());
    }

}

void WindowManagerWayland::registryHandleGlobal(void* data, wl_registry *registry,
        uint32_t name, const char* interface, uint32_t version) {
    WindowManagerWayland* self = reinterpret_cast<WindowManagerWayland*>(data);
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        // EGL apparently requires at least a version of 4 here : )
        self->compositor = (wl_compositor*)wl_registry_bind(registry, name,
                &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        self->shm = (wl_shm*)wl_registry_bind(registry, name,
                &wl_shm_interface, 1);
    } else if (strcmp(interface, wl_data_device_manager_interface.name) == 0) {
        self->deviceManager = (wl_data_device_manager*)wl_registry_bind(registry, name,
                &wl_data_device_manager_interface, 1);
    } else if (strcmp(interface,  wl_seat_interface.name) == 0) {
        self->seat = (wl_seat*)wl_registry_bind(registry, name,
                &wl_seat_interface, 5);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        wl_output* output = (wl_output*)wl_registry_bind(registry, name,
                &wl_output_interface, 2);
        Output* good = new Output(output, name);
        self->outputs.push_back(std::move(good));
    } else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0) {
        self->pointerConstraints = (zwp_pointer_constraints_v1*)wl_registry_bind(registry, name,
                &zwp_pointer_constraints_v1_interface, 1);
    } else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
        self->relativePointerManager = (zwp_relative_pointer_manager_v1*)wl_registry_bind(registry, name,
                &zwp_relative_pointer_manager_v1_interface, 1);
    } else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
        self->viewporter = (wp_viewporter*)wl_registry_bind(registry, name,
                &wp_viewporter_interface, 1);
    } else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
        self->subcompositor = (wl_subcompositor*)wl_registry_bind(registry, name,
                &wl_subcompositor_interface, 1);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        self->xdgWm = (xdg_wm_base*)wl_registry_bind(registry, name,
                &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        self->decorationManager = (zxdg_decoration_manager_v1*)wl_registry_bind(registry, name,
                &zxdg_decoration_manager_v1_interface, 1);
    } else if (strcmp(interface, xdg_activation_v1_interface.name) == 0) {
        self->xdgActivation = (xdg_activation_v1*)wl_registry_bind(registry, name,
                &xdg_activation_v1_interface, 1);
    }
}
void WindowManagerWayland::registryHandleGlobalRemove(void* data, wl_registry *registry, uint32_t name) {
    auto self = reinterpret_cast<WindowManagerWayland*>(data);
    for (std::list<Output*>::iterator it = self->outputs.begin(); it != self->outputs.end();) {
        if ((*it)->_name == name) {
            self->outputs.erase(it);
            break;
        }
        ++it;
    }
}
WindowWayland* WindowManagerWayland::getWindowForNative(wl_surface* surface) {
    if (!surface) return nullptr;
    // the tag makes it safe. Should:TM: be faster than searching a list every time
    if (!WindowWayland::ownSurface(surface))
        return nullptr;
    return reinterpret_cast<WindowWayland*>(wl_surface_get_user_data(surface));
}
void WindowManagerWayland::seatCapabilities(void* data, wl_seat* seat, uint32_t capabilities) {
    auto self = reinterpret_cast<WindowManagerWayland*>(data);
    if ((capabilities & WL_SEAT_CAPABILITY_POINTER) &&
            !self->_pointer) {
        self->_pointer.reset(new Pointer(seat, wl_seat_get_pointer(seat), self));
    } else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && self->_pointer) {
        self->_pointer.reset();
    }

    if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) &&
            !self->_keyboard) {
        self->_keyboard.reset(new Keyboard(wl_seat_get_keyboard(seat), self));
    } else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) &&
            self->_keyboard) {
        self->_keyboard.reset();
    }
}
void WindowManagerWayland::seatName(void* data, wl_seat* seat, const char* name) {
    // who cares
}
static void offerOffer(void* data, wl_data_offer* offer, const char* mimeType) {
    auto self = reinterpret_cast<WindowManagerWayland*>(data);
    self->_currentMimeTypes.push_back(std::string(mimeType));
}
wl_data_offer_listener WindowManagerWayland::_offerListener = {
    .offer = offerOffer
};
static void deviceDataOffer(void* data, wl_data_device* device, wl_data_offer* offer) {
    auto self = reinterpret_cast<WindowManagerWayland*>(data);
    self->_currentMimeTypes = {};
    wl_data_offer_add_listener(offer, &WindowManagerWayland::_offerListener, data);
}
static void deviceSelection(void* data, wl_data_device* device, wl_data_offer* offer) {
    auto self = reinterpret_cast<WindowManagerWayland*>(data);
    // if null then w/e
    self->currentOffer = offer;
}
wl_data_device_listener WindowManagerWayland::_deviceListener = {
    .data_offer = deviceDataOffer,
    .selection = deviceSelection

};
std::vector<std::string> WindowManagerWayland::getClipboardFormats() {
    return { _currentMimeTypes.begin(), _currentMimeTypes.end()};
}
jwm::ByteBuf WindowManagerWayland::getClipboardContents(const std::string& type) {
    auto it = _myClipboardContents.find(type);
    if (it != _myClipboardContents.end()) {
        return it->second;
    } else if (currentSource) {
        // Self paste
        auto it2 = _myClipboardSource.find(type);
        if (it2 != _myClipboardSource.end()) {
            _myClipboardContents[type] = it2->second;
            return it2->second;
        }
    } else if (currentOffer) {
        auto it2 = std::find(_currentMimeTypes.begin(), _currentMimeTypes.end(), type);
        if (it2 != _currentMimeTypes.end()) {
            auto mimeType = *it2;
            // pull down offer
            int fds[2];
            pipe(fds);
            wl_data_offer_receive(currentOffer, mimeType.c_str(), fds[1]);
            wl_display_flush(display);
            close(fds[1]);
            ByteBuf res;

            while (true) {
                char buf[1024];
                ssize_t n = read(fds[0], buf, sizeof(buf));
                if (n <= 0)
                    break;
                res.insert(res.end(), buf, buf + n);
            }
            // cache
            _myClipboardContents[mimeType] = res;
            close(fds[0]);
            return res;
        }
    }
    return {};
}

void WindowManagerWayland::registerWindow(WindowWayland* window) {
    _windows.push_back(window);
}

void WindowManagerWayland::unregisterWindow(WindowWayland* window) {
    auto it = std::find(_windows.begin(), _windows.end(), window);
    if (it != _windows.end()) {
        _windows.erase(it);
    }
}

void WindowManagerWayland::terminate() {
    _runLoop = false;
}

static void dataSourceSend(void* data, wl_data_source* source, const char* mimeType, int fd) {
    auto self = reinterpret_cast<WindowManagerWayland*>(data);
    auto it = self->_myClipboardSource.find(std::string(mimeType));
    if (it != self->_myClipboardSource.end()) {
        write(fd, it->second.data(), it->second.size());
    }
    close(fd);
}
static void dataSourceCancelled(void* data, wl_data_source* source) {
    auto self = reinterpret_cast<WindowManagerWayland*>(data);
    wl_data_source_destroy(source);
    self->currentSource = nullptr;
    self->_myClipboardSource = {};
}
wl_data_source_listener WindowManagerWayland::_sourceListener = {
    .send = dataSourceSend,
    .cancelled = dataSourceCancelled 
};
void WindowManagerWayland::setClipboardContents(std::map<std::string, ByteBuf>&& c) {
    _myClipboardSource = c;

    if (!deviceManager) return;

    currentSource = wl_data_device_manager_create_data_source(deviceManager);
    
    wl_data_source_add_listener(currentSource, &_sourceListener, this);

    _currentMimeTypes = {};
    for (auto it : c) {
        _currentMimeTypes.push_back(it.first.c_str());
        wl_data_source_offer(currentSource, it.first.c_str());
    }
    
    if (getKeyboardSerial() > 0)
        wl_data_device_set_selection(dataDevice, currentSource, getKeyboardSerial());

}

void WindowManagerWayland::enqueueTask(const std::function<void()>& task) {
    std::unique_lock<std::mutex> lock(_taskQueueLock);
    _taskQueue.push(task);
    _taskQueueNotify.notify_one();
    notifyLoop();
}

void WindowManagerWayland::notifyLoop() {
    // maybe just do nothing?
    if (notifyFD==-1) return;
    // fast notifyBool path to not make system calls when not necessary
    if (!notifyBool.exchange(true)) {
        char dummy[1] = {0};
        int unused = write(notifyFD, dummy, 1); // this really shouldn't fail, but if it does, the pipe should either be full (good), or dead (bad, but not our business)
    }
}
