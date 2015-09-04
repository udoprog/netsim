#include "server.h"
#include "string.h"

#include <iostream>

int main() {
    netsim::server s;

    if (!s.bind()) {
        std::cerr << "failed to bind: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    if (!s.run()) {
        std::cerr << "failed to run loop: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
