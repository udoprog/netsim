#include <fcntl.h>

#include "io.h"

bool netsim::io::setnonblocking(int fd) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
        return false;
    }

    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return false;
    }

    return true;
}
