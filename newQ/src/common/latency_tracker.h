#pragma once

#include <vector>
#include <atomic>
#include <cstdint>
#include <limits>
#include <fstream>
class LatencyTracker {
    static constexpr size_t MAX_SAMPLES = 1'000'000;

    std::vector<uint64_t> buffer;
    std::atomic<size_t> index{0};

    // histogram buckets (log scale)
    static const int NUM_BUCKETS = 64;
    std::atomic<uint64_t> buckets[NUM_BUCKETS];

public:
    LatencyTracker() : buffer(MAX_SAMPLES) {
        for (int i = 0; i < NUM_BUCKETS; i++)
            buckets[i].store(0);
    }

    // ultra-fast record (<30ns)
    void record(uint64_t latency_ns) {
        size_t i = index.fetch_add(1, std::memory_order_relaxed) % MAX_SAMPLES;
        buffer[i] = latency_ns;

        // histogram bucket (log2)
        int bucket = 0;
        uint64_t v = latency_ns;
        while (v >>= 1) bucket++;

        if (bucket >= NUM_BUCKETS) bucket = NUM_BUCKETS - 1;

        buckets[bucket].fetch_add(1, std::memory_order_relaxed);
    }

    struct Stats {
        uint64_t min;
        uint64_t max;
        uint64_t mean;
        uint64_t p50;
        uint64_t p95;
        uint64_t p99;
        uint64_t p999;
        uint64_t count;
    };

    Stats get_stats() {
        Stats s{};
        s.min = std::numeric_limits<uint64_t>::max();
        s.max = 0;
        s.mean = 0;
        s.count = std::min(index.load(), MAX_SAMPLES);

        if (s.count == 0) return s;

        // compute min, max, mean
        for (size_t i = 0; i < s.count; i++) {
            uint64_t v = buffer[i];
            if (v < s.min) s.min = v;
            if (v > s.max) s.max = v;
            s.mean += v;
        }
        s.mean /= s.count;

        // compute percentiles from histogram
        uint64_t total = 0;
        for (int i = 0; i < NUM_BUCKETS; i++)
            total += buckets[i].load();

        auto find_percentile = [&](double p) {
            uint64_t target = total * p;
            uint64_t running = 0;

            for (int i = 0; i < NUM_BUCKETS; i++) {
                running += buckets[i].load();
                if (running >= target) {
                    return (uint64_t)1 << i; // approx value
                }
            }
            return (uint64_t)1 << (NUM_BUCKETS - 1);
        };

        s.p50 = find_percentile(0.50);
        s.p95 = find_percentile(0.95);
        s.p99 = find_percentile(0.99);
        s.p999 = find_percentile(0.999);

        return s;
    }

 void export_csv(const std::string& file) {
        std::ofstream out(file);
        out << "bucket_ns,count\n";

        for (size_t i = 0; i < NUM_BUCKETS; i++) {
            out << (1ULL << i) << "," << buckets[i].load() << "\n";
        }
    }






};
