// protocol.h
#pragma once
#include <cstdint>

#pragma pack(push, 1)

struct MessageHeader {
    uint16_t type;
    uint32_t seq;
    uint64_t timestamp;
    uint16_t symbol;
};

struct Quote {
    double bid;
    uint32_t bid_qty;
    double ask;
    uint32_t ask_qty;
};

struct Trade {
    double price;
    uint32_t qty;
};

#pragma pack(pop)

enum MsgType {
    TRADE = 1,
    QUOTE = 2,
    HEARTBEAT = 3
};
