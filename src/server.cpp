#include "server.h"
#include "io.h"

#include <iostream>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/epoll.h>

bool netsim::server::bind() {
    int serverfd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (serverfd == -1) {
        return false;
    }

    if (!netsim::io::setnonblocking(serverfd)) {
        return false;
    }

    int yes = 1;

    if (::setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        ::close(serverfd);
        return false;
    }

    struct sockaddr_in inet;
    inet.sin_family = AF_INET;
    inet.sin_port = htons(1234);
    inet.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(serverfd, (struct sockaddr*)&inet, sizeof(inet)) == -1) {
        ::close(serverfd);
        return false;
    }

    if (::listen(serverfd, 32) == -1) {
        ::close(serverfd);
        return false;
    }

    int efd = ::epoll_create1(0);

    if (efd == -1) {
        ::close(serverfd);
        return false;
    }

    efd_ = efd;
    serverfd_ = serverfd;
    return true;
}

bool netsim::server::accept() {
    int clientfd;
    struct sockaddr_in peer;
    socklen_t peer_size = sizeof(peer);

    if ((clientfd = ::accept(serverfd_, reinterpret_cast<struct sockaddr*>(&peer), &peer_size)) == -1) {
        return false;
    }

    std::cerr << "accepted..." << std::endl;
    return true;
}

static const int MAXEVENTS = 64;

bool netsim::server::run() {
    struct epoll_event listen;
    listen.data.fd = serverfd_;
    listen.events = EPOLLIN;

    if (epoll_ctl(efd_, EPOLL_CTL_ADD, serverfd_, &listen)) {
        return false;
    }

    struct epoll_event events[MAXEVENTS];

    while (true) {
        int n = epoll_wait(efd_, events, MAXEVENTS, -1);

        for (int i = 0; i < n; i++) {
            int e = events[i].events;
            int fd = events[i].data.fd;

            if (fd == serverfd_) {
                if (e & EPOLLIN) {
                    accept();
                }

                continue;
            }
        }
    }

    return true;
}
