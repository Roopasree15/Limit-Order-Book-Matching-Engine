#pragma once
#include <cstdint>
#include <string>
#include <chrono>

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET };

struct Order {
    uint64_t id;
    std::string symbol;
    Side side;
    OrderType type;
    double price;          // ignored for MARKET orders
    uint32_t quantity;     // remaining quantity
    uint64_t timestamp;    // sequence number, used for price-time priority

    Order(uint64_t id_, std::string symbol_, Side side_, OrderType type_,
          double price_, uint32_t qty_, uint64_t ts_)
        : id(id_), symbol(std::move(symbol_)), side(side_), type(type_),
          price(price_), quantity(qty_), timestamp(ts_) {}
};

struct Trade {
    uint64_t tradeId;
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    std::string symbol;
    double price;
    uint32_t quantity;
    uint64_t timestamp;
};
