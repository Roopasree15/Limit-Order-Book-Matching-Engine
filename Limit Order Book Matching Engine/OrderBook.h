#pragma once
#include "Order.h"
#include <map>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

// A price-level queue: orders sitting at the same price, FIFO (time priority)
using OrderPtr = std::shared_ptr<Order>;
using PriceLevel = std::list<OrderPtr>;

// Location of an order within the book, so we can cancel in O(log n)
struct OrderLocation {
    double price;
    Side side;
    PriceLevel::iterator it;
};

class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    // Submits a new order; matches it immediately against the opposite side
    // (price-time priority), resting any unfilled remainder on the book.
    // Returns the trades generated.
    std::vector<Trade> submit(OrderPtr order);

    // Cancels a resting order by id. Returns true if it was found and removed.
    bool cancel(uint64_t orderId);

    // Best bid / ask accessors (throws std::out_of_range if empty)
    double bestBid() const;
    double bestAsk() const;
    bool hasBid() const { return !buyLevels_.empty(); }
    bool hasAsk() const { return !sellLevels_.empty(); }

    // Total resting quantity at a given price on a given side (0 if none)
    uint32_t quantityAt(Side side, double price) const;

    void printBook(std::ostream& os) const;

    const std::string& symbol() const { return symbol_; }

private:
    std::vector<Trade> match(OrderPtr& incoming);
    void rest(const OrderPtr& order);

    std::string symbol_;

    // Buy side ordered highest price first; sell side ordered lowest price first
    std::map<double, PriceLevel, std::greater<double>> buyLevels_;
    std::map<double, PriceLevel, std::less<double>> sellLevels_;

    std::unordered_map<uint64_t, OrderLocation> orderIndex_;

    uint64_t nextTradeId_ = 1;
};
