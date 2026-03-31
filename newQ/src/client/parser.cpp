#pragma once

#include "../common/protocol.h"
#include "../common/cache.h"
#include "../common/latency_tracker.h"
#include <cstring>
#include <iostream>

class Parser {
	static const size_t BUF_SIZE = 1 << 20; // 1MB buffer

	char buffer[BUF_SIZE];
	size_t read_pos = 0;
	size_t write_pos = 0;

	SymbolCache cache{100};

	uint32_t last_seq = 0;
	uint64_t gap_count = 0;

	public:
	LatencyTracker tracker;
	void on_data(const char* data, size_t len) {
		//  Prevent buffer overflow
		if (write_pos + len > BUF_SIZE) {
			std::cout << "Buffer overflow → reset\n";
			read_pos = write_pos = 0;
			return;
		}

		memcpy(buffer + write_pos, data, len);
		write_pos += len;

		parse();
	}

	SymbolCache& getCache() { return cache; }
	LatencyTracker& getTracker() { return tracker; }


	private:

bool is_valid_header(uint16_t type) {
    return type == QUOTE || type == TRADE || type == HEARTBEAT;
}
/*
bool is_valid_header(const MessageHeader& h) {
    if (!(h.type == QUOTE || h.type == TRADE || h.type == HEARTBEAT))
        return false;

    if (h.seq == 0 || h.seq > 1e9)
        return false;

    if (h.symbol >= 100)   // adjust to your symbol count
        return false;

    uint64_t now = now_ns();
    if (h.timestamp > now + 1e9)
        return false;

    return true;
}

*/





/*
void parse() {
    while (true) {
        if (write_pos - read_pos < sizeof(MessageHeader))
            break;

        MessageHeader hdr;
        memcpy(&hdr, buffer + read_pos, sizeof(MessageHeader));

        if (!is_valid_header(hdr.type)) {
            bool found = false;

            for (size_t i = read_pos + 1; i + sizeof(MessageHeader) <= write_pos; i++) {
                MessageHeader tmp;
                memcpy(&tmp, buffer + i, sizeof(MessageHeader));

                if (!is_valid_header(tmp.type))
                    continue;

                size_t test_size = sizeof(MessageHeader);

                if (tmp.type == QUOTE)
                    test_size += sizeof(Quote);
                else if (tmp.type == TRADE)
                    test_size += sizeof(Trade);

                test_size += sizeof(uint32_t);

                if (i + test_size > write_pos)
                    break;

                uint32_t recv_chk;
                memcpy(&recv_chk, buffer + i + test_size - 4, 4);

                uint32_t comp_chk = 0;
                for (size_t j = 0; j < test_size - 4; j++)
                    comp_chk ^= (uint8_t)buffer[i + j];

                if (recv_chk == comp_chk) {
                    read_pos = i;
                    found = true;
                    break;
                }
            }

            if (!found)
                read_pos = write_pos;

            continue;
        }

        size_t msg_size = sizeof(MessageHeader);

        if (hdr.type == QUOTE)
            msg_size += sizeof(Quote);
        else if (hdr.type == TRADE)
            msg_size += sizeof(Trade);

        msg_size += sizeof(uint32_t);

        if (write_pos - read_pos < msg_size)
            break;

        uint32_t received_checksum;
        memcpy(&received_checksum,
               buffer + read_pos + msg_size - 4, 4);

        uint32_t computed_checksum = 0;
        for (size_t i = 0; i < msg_size - 4; i++)
            computed_checksum ^= (uint8_t)buffer[read_pos + i];

        if (received_checksum != computed_checksum) {
            bool found = false;

            for (size_t i = read_pos + 1; i + sizeof(MessageHeader) <= write_pos; i++) {
                MessageHeader tmp;
                memcpy(&tmp, buffer + i, sizeof(MessageHeader));

                if (!is_valid_header(tmp.type))
                    continue;

                size_t test_size = sizeof(MessageHeader);

                if (tmp.type == QUOTE)
                    test_size += sizeof(Quote);
                else if (tmp.type == TRADE)
                    test_size += sizeof(Trade);

                test_size += sizeof(uint32_t);

                if (i + test_size > write_pos)
                    break;

                uint32_t recv_chk;
                memcpy(&recv_chk, buffer + i + test_size - 4, 4);

                uint32_t comp_chk = 0;
                for (size_t j = 0; j < test_size - 4; j++)
                    comp_chk ^= (uint8_t)buffer[i + j];

                if (recv_chk == comp_chk) {
                    read_pos = i;
                    found = true;
                    break;
                }
            }

            if (!found)
                read_pos = write_pos;

            continue;
        }

        if (last_seq != 0) {
            if (hdr.seq == last_seq || hdr.seq < last_seq) {
                read_pos += msg_size;
                continue;
            }
            else if (hdr.seq > last_seq + 1) {
                gap_count += (hdr.seq - last_seq - 1);
            }
        }

        process(hdr, buffer + read_pos);

        read_pos += msg_size;
    }

    if (read_pos > 0) {
        size_t remaining = write_pos - read_pos;
        memmove(buffer, buffer + read_pos, remaining);
        write_pos = remaining;
        read_pos = 0;
    }
}

*/



/*


void parse() {
    while (true) {
        if (write_pos - read_pos < sizeof(MessageHeader))
            break;

        MessageHeader hdr;
        memcpy(&hdr, buffer + read_pos, sizeof(MessageHeader));


	std::cout<<"Validate:"<<hdr.type<<std::endl;


        if (!is_valid_header(hdr.type)) {
            bool found = false;

            for (size_t i = read_pos + 1; i + sizeof(MessageHeader) <= write_pos; i++) {
                MessageHeader tmp;
                memcpy(&tmp, buffer + i, sizeof(MessageHeader));

                if (!is_valid_header(tmp.type))
                    continue;

                size_t test_size = sizeof(MessageHeader);

                if (tmp.type == QUOTE)
                    test_size += sizeof(Quote);
                else if (tmp.type == TRADE)
                    test_size += sizeof(Trade);

                test_size += sizeof(uint32_t);

                if (i + test_size > write_pos)
                    break;

                uint32_t recv_chk;
                memcpy(&recv_chk, buffer + i + test_size - 4, 4);

                uint32_t comp_chk = 0;
                for (size_t j = 0; j < test_size - 4; j++)
                    comp_chk ^= (uint8_t)buffer[i + j];

                if (recv_chk == comp_chk) {
                    if (i == read_pos)
                        read_pos += 1;
                    else
                        read_pos = i;

                    found = true;
                    break;
                }
            }

            if (!found)
                read_pos = write_pos;

            continue;
        }

        size_t msg_size = sizeof(MessageHeader);

        if (hdr.type == QUOTE)
            msg_size += sizeof(Quote);
        else if (hdr.type == TRADE)
            msg_size += sizeof(Trade);

        msg_size += sizeof(uint32_t);

        if (write_pos - read_pos < msg_size)
            break;

        uint32_t received_checksum;
        memcpy(&received_checksum,
               buffer + read_pos + msg_size - 4, 4);

        uint32_t computed_checksum = 0;
        for (size_t i = 0; i < msg_size - 4; i++)
            computed_checksum ^= (uint8_t)buffer[read_pos + i];

        if (received_checksum != computed_checksum) {
            bool found = false;

            for (size_t i = read_pos + 1; i + sizeof(MessageHeader) <= write_pos; i++) {
                MessageHeader tmp;
                memcpy(&tmp, buffer + i, sizeof(MessageHeader));

                if (!is_valid_header(tmp.type))
                    continue;

                size_t test_size = sizeof(MessageHeader);

                if (tmp.type == QUOTE)
                    test_size += sizeof(Quote);
                else if (tmp.type == TRADE)
                    test_size += sizeof(Trade);

                test_size += sizeof(uint32_t);

                if (i + test_size > write_pos)
                    break;

                uint32_t recv_chk;
                memcpy(&recv_chk, buffer + i + test_size - 4, 4);

                uint32_t comp_chk = 0;
                for (size_t j = 0; j < test_size - 4; j++)
                    comp_chk ^= (uint8_t)buffer[i + j];

                if (recv_chk == comp_chk) {
                    if (i == read_pos)
                        read_pos += 1;
                    else
                        read_pos = i;

                    found = true;
                    break;
                }
            }

            if (!found)
                read_pos = write_pos;

            continue;
        }

        if (last_seq != 0) {
            if (hdr.seq <= last_seq) {
                read_pos += msg_size;
                continue;
            } else if (hdr.seq > last_seq + 1) {
                gap_count += (hdr.seq - last_seq - 1);
            }
        }

        process(hdr, buffer + read_pos);

        read_pos += msg_size;
    }

    if (read_pos > 0) {
        size_t remaining = write_pos - read_pos;
        memmove(buffer, buffer + read_pos, remaining);
        write_pos = remaining;
        read_pos = 0;
    }
}






*/






void parse() {
    while (true) {
        if (write_pos - read_pos < sizeof(MessageHeader))
            break;

        MessageHeader hdr;
        memcpy(&hdr, buffer + read_pos, sizeof(MessageHeader));

        if (hdr.type != QUOTE &&
            hdr.type != TRADE &&
            hdr.type != HEARTBEAT) {

            bool found = false;

            for (size_t i = read_pos + 1; i + sizeof(MessageHeader) <= write_pos; i++) {
                MessageHeader tmp;
                memcpy(&tmp, buffer + i, sizeof(MessageHeader));

                if (tmp.type != QUOTE &&
                    tmp.type != TRADE &&
                    tmp.type != HEARTBEAT)
                    continue;

                size_t test_size = sizeof(MessageHeader);

                if (tmp.type == QUOTE)
                    test_size += sizeof(Quote);
                else if (tmp.type == TRADE)
                    test_size += sizeof(Trade);

                test_size += sizeof(uint32_t);

                if (i + test_size > write_pos)
                    break;

                uint32_t recv_chk;
                memcpy(&recv_chk, buffer + i + test_size - 4, 4);

                uint32_t comp_chk = 0;
                for (size_t j = 0; j < test_size - 4; j++)
                    comp_chk ^= (uint8_t)buffer[i + j];

                if (recv_chk == comp_chk) {
                    read_pos = (i == read_pos) ? read_pos + 1 : i;
                    found = true;
                    break;
                }
            }

            if (!found)
                read_pos = write_pos;

            continue;
        }

        size_t msg_size = sizeof(MessageHeader);

        if (hdr.type == QUOTE)
            msg_size += sizeof(Quote);
        else if (hdr.type == TRADE)
            msg_size += sizeof(Trade);

        msg_size += sizeof(uint32_t);

        if (write_pos - read_pos < msg_size)
            break;

        uint32_t received_checksum;
        memcpy(&received_checksum,
               buffer + read_pos + msg_size - 4, 4);

uint32_t computed_checksum =
    compute_checksum(buffer + read_pos, msg_size - sizeof(uint32_t));

        if (received_checksum != computed_checksum) {
            std::cout << "CHECKSUM FAIL"<<received_checksum<< "," <<computed_checksum<<std::endl; 
		read_pos += 1;
            continue;
        }

        if (last_seq != 0) {
            if (hdr.seq <= last_seq) {
                read_pos += msg_size;
                continue;
            }
            else if (hdr.seq > last_seq + 1) {
                gap_count += (hdr.seq - last_seq - 1);
                std::cout << "⚠️ REAL GAP: missed "
                          << (hdr.seq - last_seq - 1)
                          << " messages\n";
            }
        }

        //last_seq = hdr.seq;

        process(hdr, buffer + read_pos);

        read_pos += msg_size;
    }

    if (read_pos > 0) {
        size_t remaining = write_pos - read_pos;
        memmove(buffer, buffer + read_pos, remaining);
        write_pos = remaining;
        read_pos = 0;
    }
}




	

	uint64_t now_ns() {
		timespec ts{};
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return ts.tv_sec * 1e9 + ts.tv_nsec;
	}

	
	void process(const MessageHeader& hdr, const char* msg_ptr) {

    if (last_seq != 0 && hdr.seq != last_seq + 1) {
        gap_count++;
        std::cout << "Sequence gap detected: expected "
                  << last_seq + 1 << " got " << hdr.seq << "\n";
    }
    last_seq = hdr.seq;

    uint64_t latency = now_ns() - hdr.timestamp;
    tracker.record(latency);

    if (hdr.type == QUOTE) {
        Quote q;
        memcpy(&q, msg_ptr + sizeof(MessageHeader), sizeof(Quote));
        cache.updateQuote(hdr.symbol, q.bid, q.ask);
    }
    else if (hdr.type == TRADE) {
        Trade t;
        memcpy(&t, msg_ptr + sizeof(MessageHeader), sizeof(Trade));
        cache.updateTrade(hdr.symbol, t.price, t.qty);
    }

    static int counter = 0;
    if (++counter % 100000 == 0) {
        std::cout << "Processed 100K messages | Gaps: "
                  << gap_count << "\n";

        auto stats = tracker.get_stats();

	tracker.export_csv("../latency.csv");
        
	/*
        std::cout << "\n=== Latency Stats ===\n";
        std::cout << "Count: " << stats.count << "\n";
        std::cout << "Min: " << stats.min << " ns\n";
        std::cout << "Mean: " << stats.mean << " ns\n";
        std::cout << "Max: " << stats.max << " ns\n";
        std::cout << "p50: " << stats.p50 << " ns\n";
        std::cout << "p99: " << stats.p99 << " ns\n";
        std::cout << "p999: " << stats.p999 << " ns\n";
	*/
    }
}
	
	
	
	
	
	/*
	void process(MessageHeader* hdr) {
		//  Sequence gap detection
		if (last_seq != 0 && hdr->seq != last_seq + 1) {
			gap_count++;
			std::cout << "⚠️ Sequence gap detected: expected "
				<< last_seq + 1 << " got " << hdr->seq << "\n";
		}
		last_seq = hdr->seq;


		// Latency measurment.
		uint64_t latency = now_ns() - hdr->timestamp;
		tracker.record(latency);
		std::cout<<"here: "<<hdr->type<<std::endl;
		if (hdr->type == QUOTE) {
			auto* q = reinterpret_cast<Quote*>(hdr + 1);

			cache.updateQuote(hdr->symbol, q->bid, q->ask);
		}
		else if (hdr->type == TRADE) {
			auto* t = reinterpret_cast<Trade*>(hdr + 1);

			cache.updateTrade(hdr->symbol, t->price, t->qty);
		}

		// ⚠️ DO NOT print every message (kills performance)
		// Debug sample:
		static int counter = 0;
		if (++counter % 100000 == 0) {
			std::cout << "Processed 10K messages | Gaps: "
				<< gap_count << "\n";

			auto stats = tracker.get_stats();

			std::cout << "\n=== Latency Stats ===\n";
			std::cout << "Count: " << stats.count << "\n";
			std::cout << "Min: " << stats.min << " ns\n";
			std::cout << "Mean: " << stats.mean << " ns\n";
			std::cout << "Max: " << stats.max << " ns\n";
			std::cout << "p50: " << stats.p50 << " ns\n";
			std::cout << "p95: " << stats.p95 << " ns\n";
			std::cout << "p99: " << stats.p99 << " ns\n";
			std::cout << "p999: " << stats.p999 << " ns\n";
		}
	}
	*/


uint32_t compute_checksum(const char* data, size_t len) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum ^= (uint8_t)data[i];
    }
    return checksum;
}

/*
uint32_t compute_checksum(const char* data, size_t len) {
		uint32_t x = 0;
		for (size_t i = 0; i < len; i++) {
			x ^= static_cast<uint8_t>(data[i]);
		}
		return x;
	}

*/
};
