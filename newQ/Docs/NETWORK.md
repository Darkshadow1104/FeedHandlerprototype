 NETWORK.md
Socket Implementation Details

Overview
This document outlines the networking layer of the market data feed handler, covering:
•	TCP socket design
•	epoll-based event handling
•	message transmission strategy
•	handling of real-world TCP stream behaviour
The networking layer is designed for:
•	Low latency
•	High throughput
•	Scalability
Key Networking Principles

1. TCP is a Byte Stream
TCP does NOT preserve message boundaries
Implications:
•	Messages can arrive:
o	partially (fragmentation)
o	combined (coalescing)
•	A single recv() may contain:
o	half a message
o	multiple messages

2. Message Framing is Application Responsibility
To handle TCP correctly, the system uses:
[Header][Payload][Checksum]
The parser reconstructs messages using:
•	header-based size calculation
•	buffer accumulation

 Server-Side Networking

Socket Setup
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
Configuration:
•	AF_INET → IPv4
•	SOCK_STREAM → TCP

Binding
bind(server_fd, (sockaddr*)&addr, sizeof(addr));
•	Binds to:
o	IP: INADDR_ANY
o	Port: configurable

Listening
listen(server_fd, 1024);
•	Backlog queue supports multiple pending connections.

epoll Initialisation
epoll_fd = epoll_create1(0);

Register Server Socket
epoll_event ev{};
ev.events = EPOLLIN;
ev.data.fd = server_fd;

epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

Accepting Clients
int client_fd = accept(server_fd, ...);
Design:
•	Add client socket to epoll
•	Use non-blocking mode

epoll-Based Event Loop

Why epoll?
Feature	Benefit
O(1) scalability	handles many clients efficiently
Event-driven	avoids busy-waiting
Low overhead	minimal syscalls

Event Loop
while (true) {
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    for (int i = 0; i < n; i++) {
        if (events[i].data.fd == server_fd) {
            accept_new_client();
        } else {
            handle_client(events[i].data.fd);
        }
    }
}

Data Transmission

Server → Client
Messages are sent as:
[Header][Payload][Checksum]

Send Logic
send(client_fd, buffer, msg_size, 0);

Design Considerations
•	Small message sizes (32–44 bytes)
•	High frequency sending
•	Avoid blocking calls

Handling Partial Sends

Problem
send() may send fewer bytes than requested

Solution
Loop until all bytes are sent:
size_t sent = 0;
while (sent < msg_size) {
    ssize_t n = send(fd, buffer + sent, msg_size - sent, 0);
    sent += n;
}

Client-Side Networking

Receiving Data
ssize_t bytes = recv(fd, buffer + write_pos, capacity, 0);

Design Decisions
•	Use a continuous buffer.
•	Maintain:
o	read_pos
o	write_pos

Handling TCP Stream
Key Problem:
recv() does NOT return full messages

Solution:
•	Accumulate bytes in the buffer.
•	Parse only when the full message is available

Buffer Management

Structure
[ processed | unprocessed | free space ]

After Parsing
memmove(buffer, buffer + read_pos, remaining);

Why memmove?
•	Removes processed data
•	Keeps buffer compact
•	Avoids overflow
 Error Handling

1. Connection Closed
if (recv() == 0)
→ client disconnected

2. Non-blocking Errors
EAGAIN / EWOULDBLOCK
→ no data available

3. Invalid Data
Handled at parser level:
•	checksum failure
•	invalid header
 Flow Control (Future Work)

Problem:
Slow clients can block the server

Possible Solutions:
•	Detect send buffer full
•	drop slow clients
•	implement backpressure

Performance Optimisations

1. Non-Blocking I/O
•	Avoid thread blocking

2. epoll (Edge Triggered - future)
•	Reduce system calls

3. Disable Nagle’s Algorithm
setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, ...);

4. Buffer Sizing
setsockopt(fd, SOL_SOCKET, SO_SNDBUF, ...);
 Scalability

Current:
•	Handles multiple clients
•	High message throughput

Future Improvements:
•	multi-threaded networking
•	per-core event loops
•	kernel bypass (DPDK)

 Key Takeaways

•	TCP requires careful stream parsing
•	epoll enables scalable event-driven design
•	Buffer management is critical
•	The network layer must be non-blocking and efficient

Conclusion
The networking layer provides:
•	Reliable data transmission
•	Efficient multi-client handling
•	Low-latency communication
It is the backbone of real-time market data systems used in:
•	HFT trading platforms
•	financial data providers


