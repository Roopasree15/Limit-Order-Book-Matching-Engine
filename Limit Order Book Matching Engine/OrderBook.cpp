#include "OrderBook.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}

double OrderBook::bestBid() const {
    if (buyLevels_.empty()) throw std::out_of_range("no bids");
    return buyLevels_.begin()->first;
}

double OrderBook::bestAsk() const {
    if (sellLevels_.empty()) throw std::out_of_range("no asks");
    return sellLevels_.begin()->first;
}

uint32_t OrderBook::quantityAt(Side side, double price) const {
    uint32_t total = 0;
    if (side == Side::BUY) {
        auto lvl = buyLevels_.find(price);
        if (lvl == buyLevels_.end()) return 0;
        for (const auto& o : lvl->second) total += o->quantity;
    } else {
        auto lvl = sellLevels_.find(price);
        if (lvl == sellLevels_.end()) return 0;
        for (const auto& o : lvl->second) total += o->quantity;
    }
    return total;
}

std::vector<Trade> OrderBook::submit(OrderPtr order) {
    auto trades = match(order);
    // If a LIMIT order has leftover quantity, it rests on the book.
    // MARKET orders never rest — unfilled quantity is simply dropped (IOC-style).
    if (order->type == OrderType::LIMIT && order->quantity > 0) {
        rest(order);
    }
    return trades;
}

std::vector<Trade> OrderBook::match(OrderPtr& incoming) {
    std::vector<Trade> trades;

    auto crosses = [&](double restingPrice) {
        if (incoming->type == OrderType::MARKET) return true;
        return incoming->side == Side::BUY
                   ? incoming->price >= restingPrice
                   : incoming->price <= restingPrice;
    };

    if (incoming->side == Side::BUY) {
        while (incoming->quantity > 0 && !sellLevels_.empty() &&
               crosses(sellLevels_.begin()->first)) {
            auto levelIt = sellLevels_.begin();
            PriceLevel& level = levelIt->second;

            while (incoming->quantity > 0 && !level.empty()) {
                OrderPtr& resting = level.front();
                uint32_t fillQty = std::min(incoming->quantity, resting->quantity);

                Trade t{nextTradeId_++, incoming->id, resting->id, symbol_,
                         levelIt->first, fillQty, incoming->timestamp};
                trades.push_back(t);

                incoming->quantity -= fillQty;
                resting->quantity -= fillQty;

                if (resting->quantity == 0) {
                    orderIndex_.erase(resting->id);
                    level.pop_front();
                }
            }
            if (level.empty()) sellLevels_.erase(levelIt);
        }
    } else { // incoming SELL
        while (incoming->quantity > 0 && !buyLevels_.empty() &&
               crosses(buyLevels_.begin()->first)) {
            auto levelIt = buyLevels_.begin();
            PriceLevel& level = levelIt->second;

            while (incoming->quantity > 0 && !level.empty()) {
                OrderPtr& resting = level.front();
                uint32_t fillQty = std::min(incoming->quantity, resting->quantity);

                Trade t{nextTradeId_++, resting->id, incoming->id, symbol_,
                         levelIt->first, fillQty, incoming->timestamp};
                trades.push_back(t);

                incoming->quantity -= fillQty;
                resting->quantity -= fillQty;

                if (resting->quantity == 0) {
                    orderIndex_.erase(resting->id);
                    level.pop_front();
                }
            }
            if (level.empty()) buyLevels_.erase(levelIt);
        }
    }
    return trades;
}

void OrderBook::rest(const OrderPtr& order) {
    if (order->side == Side::BUY) {
        auto& level = buyLevels_[order->price];
        level.push_back(order);
        auto it = std::prev(level.end());
        orderIndex_[order->id] = OrderLocation{order->price, order->side, it};
    } else {
        auto& level = sellLevels_[order->price];
        level.push_back(order);
        auto it = std::prev(level.end());
        orderIndex_[order->id] = OrderLocation{order->price, order->side, it};
    }
}

bool OrderBook::cancel(uint64_t orderId) {
    auto idxIt = orderIndex_.find(orderId);
    if (idxIt == orderIndex_.end()) return false;

    const OrderLocation& loc = idxIt->second;
    if (loc.side == Side::BUY) {
        auto lvlIt = buyLevels_.find(loc.price);
        if (lvlIt != buyLevels_.end()) {
            lvlIt->second.erase(loc.it);
            if (lvlIt->second.empty()) buyLevels_.erase(lvlIt);
        }
    } else {
        auto lvlIt = sellLevels_.find(loc.price);
        if (lvlIt != sellLevels_.end()) {
            lvlIt->second.erase(loc.it);
            if (lvlIt->second.empty()) sellLevels_.erase(lvlIt);
        }
    }
    orderIndex_.erase(idxIt);
    return true;
}

void OrderBook::printBook(std::ostream& os) const {
    os << "\n--- Order Book: " << symbol_ << " ---\n";
    os << std::left << std::setw(12) << "BID QTY" << std::setw(12) << "BID PRICE"
       << " | " << std::setw(12) << "ASK PRICE" << std::setw(12) << "ASK QTY" << "\n";

    auto bIt = buyLevels_.begin();
    auto sIt = sellLevels_.begin();
    while (bIt != buyLevels_.end() || sIt != sellLevels_.end()) {
        if (bIt != buyLevels_.end()) {
            uint32_t qty = 0;
            for (auto& o : bIt->second) qty += o->quantity;
            os << std::left << std::setw(12) << qty << std::setw(12) << bIt->first;
            ++bIt;
        } else {
            os << std::setw(12) << " " << std::setw(12) << " ";
        }
        os << " | ";
        if (sIt != sellLevels_.end()) {
            uint32_t qty = 0;
            for (auto& o : sIt->second) qty += o->quantity;
            os << std::left << std::setw(12) << sIt->first << std::setw(12) << qty;
            ++sIt;
        }
        os << "\n";
    }
}
