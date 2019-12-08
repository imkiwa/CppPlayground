//
// Created by kiva on 2019/12/8.
//

#include <v9/kit/server.hpp>
#include <v9/kit/event.hpp>

using namespace v9::kit;

class IO : public EventEmitter {
};

class IO_ : public EventEmitter {
public:
    IO_() = delete;
};

class IO__ {
};

int main(int argc, const char **argv) {
    IOServer<IO> server;
    IOServer<IO_> server_;
    IOServer<IO__> server__;
}
