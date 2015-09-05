#include "socket_proxy.h"

#include <iostream>

#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <linux/un.h>

netsim::socket_proxy::socket_proxy(
        int serverfd,
        int domain,
        int type,
        int protocol,
        std::function<bool(socket_proxy*, bool)> on_writable,
        std::function<bool(socket_proxy*)> on_close
) :
    serverfd_(serverfd),
    domain_(domain),
    type_(type),
    protocol_(protocol),
    on_writable_(on_writable),
    on_close_(on_close)
{
}

int netsim::socket_proxy::connect(const struct sockaddr *addr, socklen_t addrlen)
{
    int r = 0;

    if (addr->sa_family == AF_INET) {
        const sockaddr_in* inet = reinterpret_cast<const sockaddr_in*>(addr);
        std::cerr << "inet" << std::endl;
        std::cerr << "port: " << ::ntohs(inet->sin_port) << std::endl;
        // std::cerr << "addr: " << inet->sin_addr << std::endl;
    } else {
        std::cerr << addr->sa_family << std::endl;
        std::cerr << addrlen << std::endl;
    }

    return r;
}

int netsim::socket_proxy::close()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (real_close_ == nullptr) {
        real_close_ = netsim::rtld_next<netsim::close_func_t>("close");
    }

    return real_close_(serverfd_);
}

const static int RECV_BUFFER_SIZE = 4096;

void netsim::socket_proxy::do_read() {
    char buffer[RECV_BUFFER_SIZE];

    int r = 0;

    if (type_ & SOCK_DGRAM) {
        std::cerr << "recvfrom" << std::endl;

        if ((r = ::recvfrom(serverfd_, buffer, RECV_BUFFER_SIZE, 0, NULL, 0)) < 0) {
            if (errno == EWOULDBLOCK) {
                return;
            }

            if (errno == EAGAIN) {
                return;
            }
        }
    }

    if (type_ & SOCK_STREAM) {
        if ((r = ::read(serverfd_, buffer, RECV_BUFFER_SIZE)) < 0) {
            if (errno == EWOULDBLOCK) {
                return;
            }

            if (errno == EAGAIN) {
                return;
            }

            do_except();
            return;
        }
    }

    if (r == 0) {
        return;
    }

    std::cout << "read: " << r << std::endl;
    do_except();
}

void netsim::socket_proxy::do_write() {
    int written = ::write(clientfd_, client_write_.data(), client_write_.remaining());
    client_write_.pos(client_write_.pos() + written);

    bool writable = client_write_.remaining() > 0;

    if (current_writable_ != writable) {
        on_writable_(this, writable);
        current_writable_ = writable;
    }
}

void netsim::socket_proxy::do_except() {
    std::cerr << "closing" << std::endl;
    on_close_(this);
}

void netsim::socket_proxy::readsome(std::function<size_t(char*, size_t)> reader) {
    client_write_.reset();
    size_t read = reader(client_write_.data(), client_write_.remaining());
    client_write_.limit(read);
    do_write();
}
