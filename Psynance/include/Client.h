#pragma once

#include <atomic>
#include <chrono>

template<size_t SIZE>
class RingBuffer;

class Client {
public:
    Client(const char* ip, int port);
    ~Client();

    void run();

private:
    // threads
    void receiver();
    void writer();

    // helpers
    void set_non_blocking(int fd);

private:
    int sock_;

    std::atomic<bool> running_;

    std::atomic<std::chrono::steady_clock::time_point> last_msg_;
    std::atomic<bool> waiting_pong_;
    std::atomic<std::chrono::steady_clock::time_point> ping_time_;

    RingBuffer<1024>* rb_;
};
