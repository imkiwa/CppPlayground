//
// Created by kiva on 2019/12/8.
//

#include <v9/kit/server.hpp>
#include <v9/kit/event.hpp>
#include <utility>

using namespace v9::kit;

class IO : public EventEmitter {
private:
    std::string _tag;
    int _times = 0;

public:
    IO() = default;

    ~IO() = default;

    explicit IO(std::string tag) : _tag(std::move(tag)) {}

    void log() const {
        printf("IO %p: times = %d, device = %s\n",
            this, _times, _tag.c_str());
    }

    void increment() {
        ++_times;
    }

    void openDevice(std::string device) {
        this->_tag = std::move(device);
    }
};


int main(int argc, const char **argv) {
    IOServer<std::string, IO> server;

    server.open("/dev/usb0").apply([](IO &s) {
        s.openDevice("/dev/usb0");
        s.on("data", [](const std::string &data) {
            printf("/dev/usb0: read: %s\n", data.c_str());
        });
    });

    server.open("/dev/usb0").apply([](IO &s) {
        s.increment();
        s.log();
        s.emit("data", std::string("fuck M$"));
    });
}
