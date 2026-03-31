// cache.h
#pragma once
#include <atomic>
#include<vector>

struct alignas(64) MarketState {
    std::atomic<double> bid{0};
    std::atomic<double> ask{0};
    std::atomic<double> last_price{0};
    std::atomic<double> prev_price{0};
    std::atomic<uint64_t> updates{0};
};

struct MarketSnapshot {
    double bid;
    double ask;
    double last_price;
    double prev_price; 
    uint64_t updates;
};

class SymbolCache {
    std::vector<MarketState> data;

public:
    SymbolCache(int n) : data(n) {}

    void updateQuote(int id, double bid, double ask) {
        data[id].bid.store(bid, std::memory_order_relaxed);
        data[id].ask.store(ask, std::memory_order_relaxed);
        data[id].updates.fetch_add(1, std::memory_order_relaxed);
    }

    void updateTrade(int id, double price, uint32_t qty) {
	    double old = data[id].last_price.load(std::memory_order_relaxed);
	     
	    data[id].prev_price.store(old, std::memory_order_relaxed);
	    
	    data[id].last_price.store(price, std::memory_order_relaxed);

            data[id].updates.fetch_add(1, std::memory_order_relaxed);
}
    MarketSnapshot get(int id) {
        MarketSnapshot s;
        s.bid = data[id].bid.load(std::memory_order_relaxed);
        s.ask = data[id].ask.load(std::memory_order_relaxed);
	s.last_price = data[id].last_price.load(std::memory_order_relaxed);
	s.prev_price = data[id].prev_price.load(std::memory_order_relaxed);
        s.updates = data[id].updates.load(std::memory_order_relaxed);
        return s;
    }
};
