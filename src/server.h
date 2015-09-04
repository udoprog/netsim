#pragma once

namespace netsim {
    class server {
        public:
            bool bind();

            bool run();
        private:
            bool accept();

            int serverfd_{-1};
            int efd_{-1};
    };
}
