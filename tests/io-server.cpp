//
// Created by kiva on 2019/12/8.
//

#include <v9/kit/server.hpp>
#include <v9/kit/event.hpp>

using namespace v9::kit;

class IO : public EventEmitter {
};


int main(int argc, const char **argv) {
    IOServer<std::string, IO> server;
}
