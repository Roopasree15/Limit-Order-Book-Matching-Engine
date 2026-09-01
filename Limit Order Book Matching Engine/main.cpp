#include "OrderBook.h"
#include <iostream>
#include <iomanip>

static uint64_t g_nextOrderId = 1;
static uint64_t g_clock = 1;

void printTrades(const std::vector<Trade>& trades) {
    for (const auto& t : trades) {
        std::cout << "  TRADE  id=" << t.tradeId
                  << "  buy#" << t.buyOrderId << " x sell#" << t.sellOrderId
                  << "  " << t.quantity << " @ " << std::fixed << std::setprecision(2)
                  << t.price << "\n";
    }
}

OrderPtr makeOrder(Side side, OrderType type, double price, uint32_t qty) {
    return std::make_shared<Order>(g_nextOrderId++, "INFY", side, type, price, qty, g_clock++);
}

int main() {
    OrderBook book("INFY");

    std::cout << "=== Limit Order Book & Matching Engine Demo (INFY) ===\n";

    // Build up some resting liquidity
    std::cout << "\n> Resting sell orders arrive\n";
    printTrades(book.submit(makeOrder(Side::SELL, OrderType::LIMIT, 101.50, 100)));
    printTrades(book.submit(makeOrder(Side::SELL, OrderType::LIMIT, 101.75, 200)));
    printTrades(book.submit(makeOrder(Side::SELL, OrderType::LIMIT, 101.50, 50))); // joins same price level, behind first

    std::cout << "\n> Resting buy orders arrive\n";
    printTrades(book.submit(makeOrder(Side::BUY, OrderType::LIMIT, 100.90, 150)));
    printTrades(book.submit(makeOrder(Side::BUY, OrderType::LIMIT, 100.75, 300)));

    book.printBook(std::cout);

    std::cout << "\n> Aggressive BUY LIMIT 101.60 x 120 arrives (crosses the spread)\n";
    auto trades1 = book.submit(makeOrder(Side::BUY, OrderType::LIMIT, 101.60, 120));
    printTrades(trades1);
    std::cout << "  (Fills against 101.50 level first — price priority — then price-time\n"
                 "   priority sends the remainder to the front of that level's queue)\n";

    book.printBook(std::cout);

    std::cout << "\n> MARKET SELL x 500 arrives (sweeps the book, no price limit)\n";
    auto trades2 = book.submit(makeOrder(Side::SELL, OrderType::MARKET, 0.0, 500));
    printTrades(trades2);

    book.printBook(std::cout);

    std::cout << "\n> Cancelling a resting order and confirming it's gone\n";
    auto standing = makeOrder(Side::BUY, OrderType::LIMIT, 99.00, 400);
    book.submit(standing);
    std::cout << "  Placed order #" << standing->id << " at 99.00 x 400\n";
    bool ok = book.cancel(standing->id);
    std::cout << "  Cancel result: " << (ok ? "success" : "failed") << "\n";
    std::cout << "  Quantity resting at 99.00 now: " << book.quantityAt(Side::BUY, 99.00) << "\n";

    book.printBook(std::cout);

    std::cout << "\nDone.\n";
    return 0;
}
