#pragma once

#include <mutex>
#include <thread>
#include <map>

namespace netsim {
    class socket_proxy;

    class simulator {
        public:
            simulator(int epoll_fd);
            socket_proxy* get(int sockfd);
            bool add(int clientfd, socket_proxy* s);
            bool remove(int clientfd, socket_proxy* s);
            bool writable(socket_proxy* s, bool writable);
            void loop();
        private:
            int socket_proxy_efd_;
            std::thread thread_;

            std::mutex mutex_ = {};
            volatile bool shutdown_ = false;
            std::map<int, netsim::socket_proxy*> sockets = {};
    };
}
