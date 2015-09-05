#pragma once

#include <mutex>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>

#include "dl.h"
#include "buffer.h"

namespace netsim {
    class socket_proxy {
        public:
            enum {
                BUFFER_SIZE = 4096
            };

            socket_proxy(int serverfd, int domain, int type, int protocol, std::function<bool(socket_proxy*, bool)> on_write, std::function<bool(socket_proxy*)> on_close);

            int connect(const struct sockaddr *addr, socklen_t addrlen);
            int close();

            void do_read();
            void do_write();
            void do_except();

            void readsome(std::function<size_t(char*, size_t)> reader);

            inline int serverfd() const {
                return serverfd_;
            }
        private:
            buffer client_write_{BUFFER_SIZE};
            std::mutex mutex_{};
            close_func_t real_close_{nullptr};
            bool current_writable_{false};

            int clientfd_;
            int serverfd_;
            int domain_;
            int type_;
            int protocol_;

            std::function<bool(socket_proxy*, bool)> on_writable_;
            std::function<bool(socket_proxy*)> on_close_;
    };
}
