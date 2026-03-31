// src/server/exchange_simulator.h
#pragma once
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <cstring>
#include <iostream>
#include "../common/protocol.h"
class ExchangeSimulator {
	int server_fd;
	int epoll_fd;
	std::vector<int> clients;
	uint32_t seq = 0;

	struct SymbolState {
		double price;
		double sigma;
	};

	std::vector<SymbolState> symbols;

	public:
	ExchangeSimulator(int port, size_t num_symbols = 100) {
		server_fd = socket(AF_INET, SOCK_STREAM, 0);

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;

		bind(server_fd, (sockaddr*)&addr, sizeof(addr));
		listen(server_fd, 1024);

		epoll_fd = epoll_create1(0);

		epoll_event ev{};
		ev.events = EPOLLIN;
		ev.data.fd = server_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

		std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<> dist(100, 5000);
		std::uniform_real_distribution<> vol(0.01, 0.06);


		for (int i = 0; i < num_symbols; i++) {
	//		 double price = 100 + rand() % 4900;
	//		 double sigma = 0.01 + ((double)rand() / RAND_MAX) * 0.05;

			symbols.push_back({dist(rng), vol(rng)});
	//		symbols.push_back({price, sigma});
		}
	}

	void run();
	void generate_ticks();
	uint64_t now_ns();
	void broadcast(void* data, size_t len);
        uint32_t compute_checksum(const char* data, size_t len);
	double  normal_random();
};

