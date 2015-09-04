#pragma once

namespace netsim {
    class buffer {
        public:
            buffer(size_t size) : buffer_(new char[size]), buffer_size_(size) {
            }

            char* data() {
                return buffer_ + pos_;
            }

            void reset() {
                limit_ = buffer_size_;
                pos_ = 0;
            }

            size_t pos() const {
                return pos_;
            }

            void pos(size_t pos) {
                pos_ = pos;
            }

            size_t remaining() {
                return limit_ - pos_;
            }

            void limit(size_t limit) {
                limit_ = limit;
            }

            size_t limit() const {
                return limit_;
            }
        private:
            size_t limit_ = 0;
            size_t pos_ = 0;

            char* buffer_;
            const size_t buffer_size_;
    };
}
