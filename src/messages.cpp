#include "messages.h"

#include <sstream>

namespace netsim {
namespace messages {
    connect::connect(struct sockaddr address, socklen_t address_size) :
        address(address),
        address_size(address_size)
    {
    }
}
}
