#include "simulator.h"
#include "socket_proxy.h"
#include "io.h"

#include <mutex>

#include <sys/epoll.h>

#define MAXEVENTS 64

netsim::simulator::simulator(int epoll_fd) :
    epoll_fd_(epoll_fd),
    thread_(std::bind(&netsim::simulator::loop, this))
{
}

netsim::socket_proxy* netsim::simulator::get(int sockfd) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sockets[sockfd];
}

bool netsim::simulator::add(int clientfd, netsim::socket_proxy* s) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!netsim::io::setnonblocking(s->serverfd())) {
        return false;
    }

    struct epoll_event event;
    event.data.ptr = s;
    event.events = EPOLLIN;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, s->serverfd(), &event) == -1) {
        return false;
    }

    sockets[clientfd] = s;
    return true;
}

bool netsim::simulator::remove(int clientfd, netsim::socket_proxy* s) {
    std::lock_guard<std::mutex> lock(mutex_);

    struct epoll_event event;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, s->serverfd(), &event) == -1) {
        return false;
    }

    sockets[clientfd] = s;
    return true;
}

bool netsim::simulator::writable(netsim::socket_proxy* s, bool writable) {
    std::lock_guard<std::mutex> lock(mutex_);

    struct epoll_event event;

    event.data.ptr = s;
    event.events = EPOLLIN;

    if (writable) {
        event.events |= EPOLLOUT;
    }

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, s->serverfd(), &event) == -1) {
        return false;
    }

    return true;
}

void netsim::simulator::loop() {
    struct epoll_event events[MAXEVENTS];

    while (!shutdown_) {
        int n = epoll_wait(epoll_fd_, events, MAXEVENTS, -1);

        if (n == -1) {
            break;
        }

        for (int i = 0; i < n; i++) {
            netsim::socket_proxy* s = reinterpret_cast<netsim::socket_proxy*>(events[i].data.ptr);

            int e = events[i].events;

            if ((e & EPOLLERR) || (e & EPOLLHUP)) {
                s->do_except();
                continue;
            }

            if (e & EPOLLIN) {
                s->do_read();
            }

            if (e & EPOLLOUT) {
                s->do_write();
            }
        }
    }
}

