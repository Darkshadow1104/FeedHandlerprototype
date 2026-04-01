NSE Market Data Feed Handler
Architecture & Design Decisions

Overview
This project creates a high-performance market data feed handler modelled after real-world exchange systems, such as NSE/NASDAQ.
The system simulates:
•	Exchange-side tick generation using Geometric Brownian Motion (GBM)
•	TCP-based market data distribution
•	Client-side parsing and validation of a binary protocol
•	Real-time visualization and latency tracking
The main design goals are:
•	Low latency
•	High throughput
•	Deterministic performance
•	Robust stream handling

High-Level Architecture
Exchange Simulator (Server)
        ↓
   TCP Stream (Kernel Buffer)
        ↓
Client Socket (recv)
        ↓
Stream Buffer (User-space)
        ↓
Parser (byte stream → messages)
        ↓
Processor (business logic)
        ↓
Market State Cache
        ↓
Terminal UI + Latency Tracker

Core Components

1. Exchange Simulator (Server)
Responsibilities:
•	Generate synthetic price data using GBM.
•	Maintain an independent state for each symbol.
•	Broadcast messages to multiple clients
Design Decisions:
•	Non-blocking sockets + epoll
o	Efficiently handle multiple clients.
o	Avoid blocking syscalls
•	Preallocated buffers
o	Prevent heap allocations in the hot path.
•	Binary protocol
o	Faster than JSON or text formats

2. Message Protocol
Layout:
[MessageHeader][Payload][Checksum]
Header:
struct MessageHeader {
    uint16_t type;
    uint32_t seq;
    uint64_t timestamp;
    uint16_t symbol;
};
Payload Types:
•	QUOTE → bid/ask update
•	TRADE → last traded price
•	HEARTBEAT → connection health
Design Decisions:
•	Fixed-size header ensures fast parsing.
•	XOR checksum allows lightweight validation
•	Packed structs offer a predictable memory layout.
•	No serialisation libraries keep the overhead minimal.

TCP Stream Handling
Key Insight:
TCP is a byte stream, NOT message-oriented
Problems:
•	Partial messages
•	Multiple messages in one recv
•	Fragmentation
Solution:
•	Maintain a continuous buffer.
•	Parse using header-defined message size
•	Never assume message boundaries.

Parser Design (Critical Component)
Responsibilities:
•	Extract messages from a raw byte stream.
•	Validate integrity using a checksum.
•	Handle corruption and resynchronisation.

Parsing Flow:
1. Ensure the header is available
2. Read header using memcpy
3. Determine message size
4. Check if the full message is available
5. Validate checksum
6. Process message
7. Advance read pointer

Design Decisions:
1. Safe Memory Access
memcpy(&hdr, buffer + read_pos, sizeof(MessageHeader));
✔ Avoids undefined behaviour
✔ Works with unaligned data

2. No reinterpret_cast on stream
The raw TCP buffer is not guaranteed to be aligned.

3. Checksum-first validation
Reject corrupted messages early.

4. Resynchronisation Strategy
If corruption is detected:
•	Scan forward for the next valid header.
•	Validate the checksum before accepting.
•	Prevent propagation of errors.

Market State Cache
Structure:
struct MarketState {
    std::atomic<double> bid;
    std::atomic<double> ask;
    std::atomic<double> last_price;
    std::atomic<uint32_t> updates;
};

Design Decisions:
•	Atomic variables
o	Allow lock-free updates
o	Ensure thread-safe reads/writes.
•	memory_order_relaxed
o	Keep synchronisation overhead minimal.
o	Fit for real-time data feeds
•	O(1) access per symbol
o	Use direct indexing

Processing Layer
Responsibilities:
•	Apply updates to the market state.
Logic:
Message Type	Action
QUOTE	Update bid/ask
TRADE	Update last_price (LTP)

Design Decisions:
•	Minimize branching
•	Focus on fast path execution.
•	Keep concerns separate

Terminal UI
Features:
•	Real-time updates
•	Fixed-width table layout
•	ANSI color coding:
o	Green → price up
o	Red → price down

Design Decisions:
•	Use std::setw for alignment.
•	Apply ANSI escape codes for colours.
•	Implement a full-screen refresh loop.

Latency Tracker
Requirements:
•	Store 1M samples
•	<30ns recording overhead
•	Thread-safe
•	Percentile calculation

Implementation:
•	Ring buffer
o	Use fixed memory
o	Overwrite old data
•	Atomic index
o	Allow lock-free writes
•	Histogram buckets (log scale)
o	Enable fast percentile approximation.

Metrics:
•	Min / Max / Mean
•	p50 / p95 / p99 / p999

Performance Considerations

1. Zero Dynamic Allocation
•	No new or malloc in hot paths

2. Lock-Free Design
•	No usage of mutexes
•	Only atomic operations

3. Cache Efficiency
•	Use contiguous memory
•	Keep pointer chasing minimal.

4. Binary Protocol
•	Avoid parsing overhead
•	Use direct memory operations.

5. Branch Prediction Friendly
•	Ensure predictable execution paths.

Error Handling & Robustness

1. Checksum Validation
•	Detect corrupted packets

2. Resynchronisation
•	Recover from misaligned streams.

3. Sequence Tracking
Detect:
•	duplicates
•	out-of-order messages
•	missing messages (gaps)

Scalability

Current Capability:
•	~100K messages/sec stable
•	100+ symbols supported

Future Improvements:
•	Multi-threaded parsing
•	Lock-free queues
•	Kernel bypass (DPDK)
•	NUMA optimization

Design Principles

1. Deterministic Latency
Avoid locks, allocations, and unpredictable operations.

2. Data-Oriented Design
Operate directly on raw memory.

3. Fail-Fast Validation
Reject invalid data early.

4. Throughput First
Optimise for high message rates.

Conclusion
This system shows a production-grade market data pipeline with:
•	High-performance networking
•	Strong binary stream parsing.
•	Lock-free data structures
•	Real-time visualization
•	Latency monitoring
It closely resembles architectures used in:
•	Exchange feed handlers
•	HFT trading systems
•	Real-time financial data platforms


