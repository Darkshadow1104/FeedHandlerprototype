#pragma once

#include <atomic>
#include <cstring>

// ============================================================
// Lock-Free SPSC Ring Buffer
// Single Producer (receiver thread)
// Single Consumer (writer thread)
// ============================================================

template<size_t SIZE>
class RingBuffer {
public:
    RingBuffer() : head_(0), tail_(0) {}

    // Producer (receiver thread)
 /*
    bool push(const char* data, size_t len) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next = (head + 1) % SIZE;

        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // buffer full
        }

        // copy data into buffer slot
        std::memcpy(buffer_[head], data, len);
        buffer_[head][len] = '\0'; // ensure null-termination

        head_.store(next, std::memory_order_release);
        return true;
    }
*/


    bool push(const char* data, size_t len) {
    if (len >= 128) len = 127;   // 🔥 must have

    size_t head = head_.load(std::memory_order_relaxed);
    size_t next = (head + 1) % SIZE;

    if (next == tail_.load(std::memory_order_acquire))
        return false;

    memcpy(buffer_[head], data, len);
    buffer_[head][len] = '\0';

    head_.store(next, std::memory_order_release);
    return true;
}
    // Consumer (writer thread)
    bool pop(char* out) {
        size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return false; // buffer empty
        }

        std::strcpy(out, buffer_[tail]);

        tail_.store((tail + 1) % SIZE, std::memory_order_release);
        return true;
    }

private:
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;

    // Fixed-size slots → no heap allocation in hot path
    char buffer_[SIZE][128];
};
