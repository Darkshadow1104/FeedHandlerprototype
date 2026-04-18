#pragma once

#include <unordered_map>
#include <vector>
#include <array>
#include <chrono>
#include <string>

class Server {
public:
    Server(const char* ip, int port);
    ~Server();

    void run();

private:
    struct ClientSession {
        int fd;
        std::chrono::steady_clock::time_point last_active;
        std::vector<std::array<char, 128>> out_q;
    };

    // core functions
    void setup_socket();
    void setup_epoll();
    void event_loop();

    // helpers
    void accept_clients();
    void handle_read(int fd);
    void handle_write(int fd);
    void flush_client(ClientSession& c);

    void stdin_thread();
    void enqueue_all(const char* data, int len);

    bool parse_line(const std::string& line, char* out, int& len);

    void cleanup_idle();

    void set_non_blocking(int fd);

private:
    const char* ip_;
    int port_;

    int server_fd_;
    int epfd_;

    std::unordered_map<int, ClientSession> clients_;
};
