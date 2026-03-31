#pragma once

#include "../common/cache.h"
#include "../common/latency_tracker.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iomanip>
  
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BOLD    "\033[1m"



class Visualizer {
    SymbolCache& cache;
    LatencyTracker& tracker;
    int num_symbols;

    std::vector<std::string> symbols = {
    "RELIANCE","TCS","INFY","HDFCBANK","ICICIBANK","SBIN","ITC","LT","HCLTECH","WIPRO",
    "AXISBANK","KOTAKBANK","MARUTI","TITAN","ULTRACEMCO","SUNPHARMA","BAJFINANCE","NTPC","ONGC","POWERGRID",
    "ADANIPORTS","ADANIENT","ASIANPAINT","NESTLEIND","HINDUNILVR","BRITANNIA","TECHM","COALINDIA","DIVISLAB","DRREDDY",
    "CIPLA","EICHERMOT","GRASIM","HEROMOTOCO","HDFCLIFE","SBILIFE","JSWSTEEL","TATASTEEL","INDUSINDBK","BAJAJFINSV",
    "APOLLOHOSP","DABUR","GODREJCP","PIDILITIND","SIEMENS","ABB","BHEL","BPCL","IOC","GAIL",
    "AMBUJACEM","ACC","VEDL","SAIL","NMDC","HINDALCO","TATAMOTORS","M&M","ESCORTS","ASHOKLEY",
    "PAGEIND","COLPAL","MARICO","UBL","MCDOWELL","ZYDUSLIFE","TORNTPHARM","LUPIN","AUROPHARMA","ALKEM",
    "BANKBARODA","PNB","CANBK","IDFCFIRSTB","FEDERALBNK","RBLBANK","YESBANK","BANDHANBNK","INDIANB","CENTRALBK",
    "IRCTC","ZOMATO","NYKAA","PAYTM","POLICYBZR","NAUKRI","DELHIVERY","DMART","TRENT","VBL",
    "ADANIGREEN","ADANIPOWER","TATAPOWER","NHPC","SUZLON","KPITTECH","PERSISTENT","COFORGE","LTIM","MPHASIS"
};
public:
    Visualizer(SymbolCache& c, LatencyTracker& t, int n)
        : cache(c), tracker(t), num_symbols(n) {}

    void run() {
        while (true) {
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

private:
    void clear_screen() {
        std::cout << "\033[2J\033[H"; // ANSI clear
    }

    void render() {
        clear_screen();

        std::cout << "=== NSE Market Data Feed Handler ===\n";

        //  Latency stats
  /*
  	auto stats = tracker.get_stats();

        std::cout << "Latency(ns): "
                  << "p50=" << stats.p50
                  << " p99=" << stats.p99
                  << " p999=" << stats.p999
                  << "\n\n";
*/
        //  Collect top symbols by updates
        struct Row {
            int id;
            MarketSnapshot s;
        };

        std::vector<Row> rows;

        for (int i = 0; i < num_symbols; i++) {
            auto snap = cache.get(i);
            rows.push_back({i, snap});
        }

        // sort by updates
        std::sort(rows.begin(), rows.end(),
                  [](const Row& a, const Row& b) {
                      return a.s.updates > b.s.updates;
                  });

        //std::cout << "Symbol        Bid        Ask        LTP        Updates\n";

std::cout << std::fixed << std::setprecision(2);
	std::cout << BOLD << YELLOW
          << std::left << std::setw(12) << "Symbol"
          << std::right << std::setw(12) << "Bid"
          << std::setw(12) << "Ask"
          << std::setw(12) << "LTP"
          << std::setw(12) << "Updates"
          << RESET << "\n";
	
	
	std::cout << "------------------------------------------------------------\n";

        int limit = std::min(20, (int)rows.size());

        for (int i = 0; i < limit; i++) {
            auto& r = rows[i];


	    std::string color = YELLOW;

	    if (r.s.last_price > r.s.prev_price)
		    color = GREEN;
	    else if (r.s.last_price < r.s.prev_price)
		    color = RED;


           std::cout << std::left << std::setw(12) << symbols[r.id]
          << std::right << std::setw(12) << std::fixed << std::setprecision(2) << GREEN <<r.s.bid << RESET
          << std::setw(12) <<  RED <<r.s.ask << RESET
          << std::setw(12) << color <<  r.s.last_price << RESET
          << std::setw(12) <<  r.s.updates
          << "\n";

/*
	    std::cout << symbols[r.id] << "              "
                      << GREEN <<r.s.bid << RESET <<"    "
                      << RED <<r.s.ask << RESET <<"    "
		      << color << r.s.last_price << RESET <<"     " 
                      << r.s.updates << "\n";
	    */
        }

        std::cout << "\nPress Ctrl+C to exit\n";
    }
};
