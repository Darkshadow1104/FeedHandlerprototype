#include "../include/Client.h"
#include "../include/RingBuffer.h"

#include <iostream>
#include <thread>
#include <cstring>

#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

// ================= CONSTRUCTOR =================
Client::Client(const char* ip, int port)
    : sock_(-1),
      running_(true),
      waiting_pong_(false)
{
    rb_ = new RingBuffer<1024>();

    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    set_non_blocking(sock_);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    connect(sock_, (sockaddr*)&addr, sizeof(addr));

    last_msg_ = std::chrono::steady_clock::now();
}

// ================= DESTRUCTOR =================
Client::~Client() {
    if (sock_ >= 0) close(sock_);
    delete rb_;
}

// ================= NON-BLOCK =================
void Client::set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// ================= RECEIVER THREAD =================
void Client::receiver() {
	char buffer[BUFFER_SIZE];

	while (running_) {
		int n = recv(sock_, buffer, sizeof(buffer), 0);
		if (n < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
		} 
		auto now = std::chrono::steady_clock::now();


		if (n > 0) {
			int start = 0;

			for (int i = 0; i < n; i++) {
				if (buffer[i] == '\n') {
					int len = i - start + 1;

					// Handle PONG separately
					if (len >= 4 && strncmp(buffer + start, "PONG", 4) == 0) {
						waiting_pong_ = false;
					} else {
						rb_->push(buffer + start, len);
					}

					start = i + 1;
				}
			}

			last_msg_ = now;
		}		

/*		
		if (n > 0) {
			if (strncmp(buffer, "PONG", 4) == 0) {
				waiting_pong_ = false;
			} else {
				rb_->push(buffer, n);
			}
			last_msg_ = now;
		}
*/
		// ---------- HEARTBEAT ----------
		auto idle = std::chrono::duration_cast<std::chrono::seconds>(
				now - last_msg_.load());

		if (idle.count() >= 30 && !waiting_pong_) {
			send(sock_, "PING\n", 5, 0);
			waiting_pong_ = true;
			ping_time_ = now;
		}

		if (waiting_pong_) {
			auto diff = std::chrono::duration_cast<std::chrono::seconds>(
					now - ping_time_.load());

			if (diff.count() >= 5) {
				std::cout << "Server Crashed\n";
				running_ = false;
				close(sock_);
				break;
			}
		}
	}
}

// ================= WRITER THREAD =================
void Client::writer() {
    char msg[128];

    while (running_) {
        if (rb_->pop(msg)) {
            write(STDOUT_FILENO, msg, strlen(msg));
        }
    }
}

// ================= RUN =================
void Client::run() {
    std::thread t1(&Client::receiver, this);
    std::thread t2(&Client::writer, this);

    t1.join();
    t2.join();
}
