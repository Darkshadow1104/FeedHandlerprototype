// exchange_simulator.cpp
#include "exchange_simulator.h"
#include <fcntl.h>
void ExchangeSimulator::run() {
	epoll_event events[1024];

	while (true) {
		int n = epoll_wait(epoll_fd, events, 1024, 0);

		for (int i = 0; i < n; i++) {
			if (events[i].data.fd == server_fd) {
				int client = accept(server_fd, nullptr, nullptr);
				if (client >= 0) {
					fcntl(client, F_SETFL, O_NONBLOCK);
					clients.push_back(client);
				} 
				//clients.push_back(client);
			}
		}

		generate_ticks();
	}
}




double  ExchangeSimulator::normal_random() {
	static bool hasSpare = false;
	static double spare;

	if (hasSpare) {
		hasSpare = false;
		return spare;
	}

	hasSpare = true;

	double u, v, s;
	do {
		u = (double)rand() / RAND_MAX * 2.0 - 1.0;
		v = (double)rand() / RAND_MAX * 2.0 - 1.0;
		s = u*u + v*v;
	} while (s >= 1 || s == 0);

	s = sqrt(-2.0 * log(s) / s);
	spare = v * s;

	return u * s;
}



void  ExchangeSimulator::generate_ticks() {
	for (int i = 0; i < symbols.size(); i++) {
		auto &s = symbols[i];

		double dt = 0.001;   //1 ms
		double mu = 0.0;     //neutral market.

		double dW = ((double)rand()/RAND_MAX - 0.5);

                // GBM price update
		

		double Z = normal_random();
		s.price *= exp((mu - 0.5 * s.sigma * s.sigma) * dt +
				s.sigma * sqrt(dt) * Z);
		if (s.price < 1.0) s.price = 1.0;


		//s.price += mu* + s.price * s.sigma * dW;
		//s.price *= exp((mu - 0.5 * s.sigma * s.sigma) * dt +
		//		s.sigma * sqrt(dt) * dW);

		alignas(64) char buffer[128] = {0};


/*

		std::cout << sizeof(MessageHeader) << " "
          << sizeof(Quote) << " "
          << sizeof(Trade) << "\n";
*/
/*
		MessageHeader *hdr = (MessageHeader*)buffer;
		//auto* hdr = reinterpret_cast<MessageHeader*>(buffer);
		//hdr->type = QUOTE;
		hdr->seq = ++seq;
		hdr->timestamp = now_ns();
		hdr->symbol = i;
*/
		MessageHeader hdr{};
//hdr.type = QUOTE;
hdr.seq = ++seq;
hdr.timestamp = now_ns();
hdr.symbol = i;
		size_t msg_size = sizeof(MessageHeader);
  
	       size_t offset = 0;
		// 70% Quote, 30% Trade


		if (rand() % 100 < 70) {
			hdr.type = QUOTE;

                // realistic spread (0.05%–0.2%)
 
			double spread = s.price * (0.0005 + ((double)rand()/RAND_MAX) * 0.0015);
			Quote q{};
			q.bid = s.price - spread / 2;
			q.ask = s.price + spread / 2;
			q.bid_qty = 100 + rand() % 1000;
			q.ask_qty = 100 + rand() % 1000;

			memcpy(buffer + offset, &hdr, sizeof(hdr));
			
			offset +=sizeof(hdr);

			//memcpy(buffer + sizeof(hdr), &q, sizeof(q));
			memcpy(buffer + offset, &q, sizeof(q));
                        
			offset+=sizeof(q);

			msg_size += sizeof(Quote);

		}

		else {
			//hdr->type = TRADE;
			hdr.type = TRADE;

			Trade t{};
			t.price = s.price;
			t.qty = 100 + rand() % 1000;

			memcpy(buffer + offset, &hdr, sizeof(hdr));
			
			 offset +=sizeof(hdr);

			//memcpy(buffer + sizeof(hdr), &t, sizeof(t));
			memcpy(buffer + offset, &t, sizeof(t));
                         
			  offset+=sizeof(t);
	
			msg_size += sizeof(Trade);
		}



		/*

		//Quote *q = (Quote*)(buffer + sizeof(MessageHeader));
		auto* q = reinterpret_cast<Quote*>(buffer + sizeof(MessageHeader));
		q->bid = s.price * 0.999;
		q->ask = s.price * 1.001;
		q->bid_qty = 100;
		q->ask_qty = 100;

		size_t msg_size = sizeof(MessageHeader) + sizeof(Quote);
		*/




		//comput the checksum
		//uint32_t checksum = compute_checksum(buffer, msg_size);
		uint32_t checksum = compute_checksum(buffer, offset);



		//append checksum
		//memcpy(buffer + msg_size, &checksum, sizeof(uint32_t));
		memcpy(buffer + offset, &checksum, sizeof(uint32_t));

                    offset+=sizeof(uint32_t);

		msg_size += sizeof(uint32_t);


		//broadcast(buffer, msg_size);
		broadcast(buffer, offset);
		//broadcast(buffer, sizeof(MessageHeader) + sizeof(Quote));
	}
}

uint64_t  ExchangeSimulator::now_ns() {
	timespec ts{};
	//clock_gettime(CLOCK_REALTIME, &ts);
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void  ExchangeSimulator::broadcast(void* data, size_t len) {
//		 std::cout << "Sending message of size: " << len << "\n";
	//	for (int fd : clients) {
	//		send(fd, data, len, MSG_DONTWAIT);
	//	}
	//updated one.

	for (auto it = clients.begin(); it != clients.end(); ) {
		int fd = *it;

		ssize_t n = send(fd, data, len, MSG_DONTWAIT);

		if (n < 0) {
			if (errno == EPIPE || errno == ECONNRESET) {
				std::cout << "Client disconnected: " << fd << "\n";

				close(fd);
				it = clients.erase(it);  // ✅ remove dead client
				continue;
			}
		}

		++it;
	}
}

uint32_t ExchangeSimulator::compute_checksum(const char* data, size_t len) {
    uint32_t x = 0;
    for (size_t i = 0; i < len; i++) {
        x ^= static_cast<uint8_t>(data[i]);
    }
    return x;
}
