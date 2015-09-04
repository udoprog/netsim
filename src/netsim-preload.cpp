#include <string>
#include <cstddef>
#include <map>
#include <mutex>
#include <thread>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "socket_proxy.h"
#include "dl.h"
#include "simulator.h"

static netsim::connect_func_t real_connect = nullptr;
static netsim::socket_func_t real_socket = nullptr;

static netsim::simulator* get_or_setup() {
    static netsim::simulator* sim = nullptr;
    static std::mutex lib_mutex = {};

    if (sim != nullptr) {
        return sim;
    }

    std::lock_guard<std::mutex> lock(lib_mutex);

    real_connect = netsim::rtld_next<netsim::connect_func_t>("connect");
    real_socket = netsim::rtld_next<netsim::socket_func_t>("socket");

    // check again, might have been setup.
    if (sim != nullptr) {
        return sim;
    }

    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        ::exit(127);
    }

    return (sim = new netsim::simulator(epoll_fd));
}

extern "C" {
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        netsim::simulator* sim = get_or_setup();

        netsim::socket_proxy* s;

        if ((s = sim->get(sockfd)) != nullptr) {
            ::fprintf(stderr, "connect(...)\n");
            return s->connect(addr, addrlen);
        }

        return real_connect(sockfd, addr, addrlen);
    }

    int socket(int domain, int type, int protocol) {
        if (!(domain == AF_INET || domain == AF_INET6)) {
            return real_socket(domain, type, protocol);
        }

        netsim::simulator* sim = get_or_setup();

        // open fake sockfd.
        int sv[2] = {-1, -1};

        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
            return -1;
        }

        int clientfd = sv[0], serverfd = sv[1];

        auto on_writable = std::bind(&netsim::simulator::writable, sim, std::placeholders::_1, std::placeholders::_2);
        auto on_close = std::bind(&netsim::simulator::remove, sim, clientfd, std::placeholders::_1);

        netsim::socket_proxy* s = new netsim::socket_proxy(serverfd, domain, type, protocol, on_writable, on_close);

        if (!sim->add(clientfd, s)) {
            delete s;
            return -1;
        }

        return clientfd;
    }
}
