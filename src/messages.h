#pragma once

#include <string>
#include <sys/socket.h>

#include <istream>
#include <ostream>
#include <string>

namespace netsim {
    namespace messages {
        template<typename T>
        class serialized {
            public:
                static void serialize(T& object, std::ostream out);
                static T deserialize(std::istream in);
        };

        class connect {
            public:
                connect(struct sockaddr address, socklen_t address_size);

                friend class serialized<connect>;
            private:
                const struct sockaddr address;
                const socklen_t address_size;
        };

        template <>
        class serialized<connect> {
            public:
                static connect deserialize(std::istream& in) {
                    socklen_t address_size;
                    struct sockaddr addr;

                    in.read(reinterpret_cast<char*>(&address_size), sizeof(socklen_t));
                    in.read(reinterpret_cast<char*>(&addr), address_size);

                    return connect(addr, address_size);
                }

                static void serialize(connect& object, std::ostream& out) {
                    out.write(reinterpret_cast<const char*>(&object.address_size), sizeof(socklen_t));
                    out.write(reinterpret_cast<const char*>(&object.address), object.address_size);
                }
        };

        class send_data {
            public:
                send_data(std::string data);

                friend class serialized<send_data>;
            private:
                const std::string data;
        };

        template <>
        class serialized<send_data> {
            public:
                static send_data deserialize(std::istream& in) {
                    size_t size;

                    in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
                    std::string data(size, '\0');
                    in.read(&data[0], data.size());

                    return send_data(data);
                }

                static void serialize(send_data& object, std::ostream& out) {
                    size_t size = object.data.size();

                    out.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
                    out.write(reinterpret_cast<const char*>(object.data.c_str()), object.data.size());
                }
        };
    }
}
