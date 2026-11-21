#ifndef NANO_EDR_RING_BUFFER_H
#define NANO_EDR_RING_BUFFER_H
#include <cstddef>
namespace nano_edr {
template <typename T, std::size_t N>
class RingBuffer;
template <typename T, std::size_t N>
class RingBufferIterator {
 public:
    const T& operator*() const {
        return (*buffer_)[index_];
    }

    RingBufferIterator& operator++() {
        ++index_;
        return *this;
    }

    bool operator!=(const RingBufferIterator& other) const {
        return buffer_ != other.buffer_ || index_ != other.index_;
    }

 private:
    const RingBuffer<T, N>* buffer_ = nullptr;
    std::size_t index_ = 0;
    friend class RingBuffer<T, N>;
    RingBufferIterator(const RingBuffer<T, N>* buffer, std::size_t index)
        : buffer_(buffer), index_(index) {}
};
template <typename T, std::size_t N>
class RingBuffer {
 public:
    using Iterator = RingBufferIterator<T, N>;
    void PushBack(const T& value) {
        data_[cur_pos] = value;
        cur_pos = (cur_pos + 1) % N;
        if (size_ < N) {
            size_++;
        }
    }
    void Clear() {
        size_ = 0;
        cur_pos = 0;
    }
    std::size_t size() const {
        return size_;
    }
    static constexpr std::size_t capacity() {
        return N;
    }
    bool empty() const {
        return size_ == 0;
    }
    bool full() const {
        return size_ == N;
    }
    const T& operator[](std::size_t index) const {
        std::size_t start = (cur_pos + N - size_) % N;
        return data_[(start + index) % N];
    }
    const T& back() const {
        return (*this)[size_ - 1];
    }
    Iterator begin() const {
        return Iterator(this, 0);
    }
    Iterator end() const {
        return Iterator(this, size_);
    }
 private:
    T data_[N]{};
    std::size_t size_ = 0;
    std::size_t cur_pos = 0;
};
}
#endif