#pragma once

#include <dlfcn.h>

namespace netsim {
    typedef int (*close_func_t)(int fd);
    typedef int (*connect_func_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    typedef int (*socket_func_t)(int domain, int type, int protocol);

    template<typename T>
    static T rtld_next(const std::string symbol) {
        T reference = reinterpret_cast<T>(::dlsym(RTLD_NEXT, symbol.c_str()));

        if (reference == nullptr) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
            ::exit(127);
        }

        return reference;
    }
}
