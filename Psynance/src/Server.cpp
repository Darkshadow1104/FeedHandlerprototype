#include "../include/Server.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/uio.h>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 128
#define TIMEOUT 60

static std::mutex mtx;

// ---------- constructor ----------
Server::Server(const char* ip, int port)
    : ip_(ip), port_(port), server_fd_(-1), epfd_(-1) {}

Server::~Server() {}

// ---------- utils ----------
void Server::set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// ---------- setup ----------
void Server::setup_socket() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    set_non_blocking(server_fd_);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    inet_pton(AF_INET, ip_, &addr.sin_addr);

    bind(server_fd_, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd_, 128);
}

void Server::setup_epoll() {
    epfd_ = epoll_create1(0);

    epoll_event ev{};
    //ev.events = EPOLLIN | EPOLLET;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd_;

    epoll_ctl(epfd_, EPOLL_CTL_ADD, server_fd_, &ev);
}

// ---------- parsing ----------
bool Server::parse_line(const std::string& line, char* out, int& len) {
    int t, q;
    double p;

    if (sscanf(line.c_str(), "%d,%lf,%d", &t, &p, &q) != 3)
        return false;

    std::cout<<t<<","<<p<<","<<q<<std::endl;
    if (t < 1 || t > 10 || p < 100 || p > 200 || q < 10 || q > 50)
        return false;
    

    len = snprintf(out, BUFFER_SIZE, "%d,%.2f,%d\n", t, p, q);
    std::cout<<"nitesh7:"<<len<<std::endl;

    return true;
}

// ---------- enqueue ----------
void Server::enqueue_all(const char* data, int len) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout<<"nitesh8"<<std::endl;
    std::cout << "Connected clients: " << clients_.size() << std::endl;
    for (auto& [fd, c] : clients_) {
        std::array<char, BUFFER_SIZE> msg{};
        
    std::cout<<"nitesh9"<<std::endl;
	memcpy(msg.data(), data, len);
        
    std::cout<<"nitesh10"<<std::endl;
	c.out_q.push_back(msg);
	std::cout << "Broadcasting: " << data;
    }
}

// ---------- stdin ----------
void Server::stdin_thread() {
    while (clients_.empty()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
	std::string line;
    char buffer[BUFFER_SIZE];
    int len;

    while (std::getline(std::cin, line)) {
	    std::cout<<"line:"<<line<<std::endl;
	    if (parse_line(line, buffer, len)) {
		    std::cout<<"nitesh6"<<std::endl;
            enqueue_all(buffer, len);
        }
    }
}

// ---------- accept ----------
void Server::accept_clients() {
    while (true) {
        int cfd = accept(server_fd_, nullptr, nullptr);
        if (cfd < 0) break;

        set_non_blocking(cfd);

        epoll_event ev{};
        //ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.events = EPOLLIN | EPOLLOUT;
        ev.data.fd = cfd;

        epoll_ctl(epfd_, EPOLL_CTL_ADD, cfd, &ev);

        std::lock_guard<std::mutex> lock(mtx);
        clients_[cfd] = {cfd, std::chrono::steady_clock::now()};
	std::cout << "Client connected: fd=" << cfd << std::endl;
    }
}

// ---------- read ----------
void Server::handle_read(int fd) {
    char buf[64];
/*
    int r = recv(fd, buf, sizeof(buf), 0);

    if (r <= 0) {
        close(fd);
        clients_.erase(fd);
        return;
    }
std::cout << "Received PING from fd=" << fd << std::endl;
    if (strncmp(buf, "PING", 4) == 0) {
        send(fd, "PONG\n", 5, 0);
    }

    clients_[fd].last_active = std::chrono::steady_clock::now();
*/


    while (true) {
    int r = recv(fd, buf, sizeof(buf), 0);
    if (r <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            break;

        close(fd);
        clients_.erase(fd);
        break;
    }

    std::cout<<"RECEIVING PONG: "<<r<<std::endl;
    if (strncmp(buf, "PING", 4) == 0) {
        send(fd, "PONG\n", 5, 0);
    }

    clients_[fd].last_active = std::chrono::steady_clock::now();
}
    }

// ---------- write ----------
void Server::flush_client(ClientSession& c) {
	if (c.out_q.empty()) return;

	struct iovec iov[16];
	int count = 0;

	for (; count < (int)c.out_q.size() && count < 16; count++) {
		iov[count].iov_base = c.out_q[count].data();
		iov[count].iov_len  = strlen(c.out_q[count].data());
	}

	ssize_t n = writev(c.fd, iov, count);

	if (n < 0) {
		if (errno == EAGAIN) return;
		close(c.fd);
		return;
	}

	// Remove ONLY what was actually sent
	size_t bytes_sent = n;

	while (bytes_sent > 0 && !c.out_q.empty()) {
		size_t msg_len = strlen(c.out_q.front().data());

		if (bytes_sent >= msg_len) {
			bytes_sent -= msg_len;
			c.out_q.erase(c.out_q.begin());
		} else {
			// partial message (rare for small msgs)
			break;
		}
	}
	/*
	   if (n > 0) {
	   c.out_q.erase(c.out_q.begin(), c.out_q.begin() + count);
	   c.last_active = std::chrono::steady_clock::now();

	   }
	   */
}

void Server::handle_write(int fd) {
    flush_client(clients_[fd]);
}

// ---------- cleanup ----------
void Server::cleanup_idle() {
    auto now = std::chrono::steady_clock::now();

    for (auto it = clients_.begin(); it != clients_.end();) {
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.last_active);

        if (diff.count() > TIMEOUT) {
            close(it->first);
            it = clients_.erase(it);
        } else {
            ++it;
        }
    }
}

// ---------- loop ----------
void Server::event_loop() {
	epoll_event events[MAX_EVENTS];

	while (true) {
		int n = epoll_wait(epfd_, events, MAX_EVENTS, 1000);

		for (int i = 0; i < n; i++) {
			int fd = events[i].data.fd;

			if (fd == server_fd_) {
				accept_clients();
			} else {
				std::lock_guard<std::mutex> lock(mtx);

				if (clients_.find(fd) == clients_.end()) continue;

				if (events[i].events & EPOLLIN)
					handle_read(fd);

				if (events[i].events & EPOLLOUT)
					handle_write(fd);
			}

		}


		std::lock_guard<std::mutex> lock(mtx);

		for (auto& [fd, c] : clients_) {
			if (!c.out_q.empty()) {
				handle_write(fd);
			}
		}

		cleanup_idle();
	}
}

// ---------- run ----------
void Server::run() {

	std::cout<<"niteh1"<<std::endl;
    setup_socket();

	std::cout<<"niteh2"<<std::endl;
    setup_epoll();

	std::cout<<"niteh3"<<std::endl;
    std::thread(&Server::stdin_thread, this).detach();

	std::cout<<"niteh4"<<std::endl;
    event_loop();

	std::cout<<"niteh5"<<std::endl;
}
