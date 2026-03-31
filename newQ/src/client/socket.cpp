// socket.cpp
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include<string>
#include<cstring>
class MarketDataSocket {
    int sock;

public:
    bool connect(const std::string& host, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        return ::connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0;
    }

    ssize_t receive(void* buf, size_t len) {
        return recv(sock, buf, len, 0);
    }

    int fd() const { return sock; }
};
