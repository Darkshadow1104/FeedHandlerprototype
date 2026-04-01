QUESTIONS.md
Answers to Critical Thinking Questions
________________________________________
1. Broadcasting and Client Handling
1.1 How do you efficiently broadcast to multiple clients without blocking?
Use non-blocking sockets with an event-driven model (epoll). Each client is handled independently. When sending data:
•	Attempt to send immediately
•	If partial send occurs, store remaining data in a per-client buffer
•	Do not block on any client
This ensures one slow client does not affect others.
________________________________________
1.2 What happens when a client's TCP send buffer fills up?
The send() call returns EAGAIN or EWOULDBLOCK.
This indicates the kernel cannot accept more data. The system should:
•	Mark the client as temporarily unwritable
•	Stop sending further data to that client
•	Resume when EPOLLOUT event is triggered
________________________________________
1.3 How do you ensure fair distribution when some clients are slower?
Use per-client buffers and isolate slow clients:
•	Each client maintains its own outgoing queue
•	Slow clients do not block fast ones
•	Optionally disconnect clients that cannot keep up
________________________________________
1.4 How would you handle 1000+ concurrent client connections?
Use:
•	epoll for O(1) scalability
•	non-blocking sockets
•	avoid per-client threads
For further scaling:
•	use multiple event loops (per core)
•	use SO_REUSEPORT
________________________________________
2. epoll and Low-Level Networking
2.1 Why use epoll edge-triggered instead of level-triggered?
Edge-triggered mode reduces repeated notifications and system calls.
•	Level-triggered repeatedly signals readiness
•	Edge-triggered signals only on state change
This improves performance in high-throughput systems.
________________________________________
2.2 How do you handle recv() returning EAGAIN or EWOULDBLOCK?
This indicates no more data is available.
•	Stop reading
•	Return control to the event loop
•	Wait for the next EPOLLIN event
________________________________________
2.3 What happens if the kernel receive buffer fills up?
The TCP receive window shrinks, causing the sender to slow down.
Effects:
•	Increased latency
•	Reduced throughput
________________________________________
2.4 How do you detect a silent connection drop?
Use:
•	TCP keepalive
•	Application-level heartbeats
If no data or heartbeat is received within a timeout window, consider the connection dead.
________________________________________
3. Parser and Stream Handling
3.1 How do you buffer incomplete messages across multiple recv() calls?
Maintain a continuous buffer with:
•	read_pos
•	write_pos
Append incoming data and parse only when a full message is available.
________________________________________
3.2 What happens when you detect a sequence gap?
Do not request retransmission.
•	Log the gap
•	Continue processing
Real-time systems prioritize latency over completeness.
________________________________________
3.3 How do messages arrive out of order if TCP guarantees order?
TCP guarantees ordering per connection. Out-of-order scenarios may arise from:
•	multi-threaded processing
•	merging multiple data streams
•	replay or recovery mechanisms
________________________________________
3.4 How do you prevent buffer overflow with malicious message lengths?
Validate message size:
•	Reject messages exceeding a maximum threshold
•	Validate header fields
•	Ensure bounds checking before parsing
________________________________________
4. Concurrency and Memory
4.1 How do you prevent readers from seeing inconsistent state?
Use atomic variables for shared data.
This guarantees readers always observe a valid value.
________________________________________
4.2 What memory ordering is required?
Use memory_order_relaxed:
•	Only atomicity is required
•	No strict ordering between operations
________________________________________
4.3 How do you handle cache line bouncing?
Minimize shared writes:
•	Use per-symbol data structures
•	Align or pad structures if necessary
________________________________________
4.4 Do you need Read-Copy-Update (RCU)?
No.
The system uses simple overwrite semantics with atomic variables, so RCU is unnecessary.
________________________________________
5. UI and Display
5.1 How do you update display without affecting network threads?
Keep UI separate from the hot path:
•	run UI in a separate thread, or
•	update periodically at a lower frequency
________________________________________
5.2 Should you use ncurses or ANSI codes?
Use ANSI escape codes:
•	lightweight
•	minimal overhead
•	sufficient for simple terminal UI
ncurses adds unnecessary complexity.
________________________________________
5.3 How do you calculate percentage change?
Use:
change = (current_price - reference_price) / reference_price * 100
Typically reference_price is previous close.
________________________________________
6. Latency and Performance
6.1 How do you calculate percentiles faster than sorting?
Use histogram-based approximation:
•	O(1) insertion
•	O(buckets) query
Avoid sorting large datasets.
________________________________________
6.2 How do you minimize timestamping overhead?
Use:
•	clock_gettime(CLOCK_MONOTONIC)
•	or CPU timestamp counter (rdtsc)
Avoid expensive system calls in hot path.
________________________________________
6.3 What histogram granularity should be used?
Use log-scale buckets (powers of 2):
•	balances accuracy and memory usage
•	suitable for wide latency ranges
________________________________________
7. System Design
7.1 Should reconnection logic be in the same thread?
No.
Reconnection should be handled in a separate thread:
•	avoids blocking main processing
•	improves system responsiveness
________________________________________
Conclusion
The system is designed with:
•	non-blocking networking
•	lock-free data structures
•	efficient parsing
•	robust error handling
These principles ensure low latency, high throughput, and stability in real-time market data systems.


