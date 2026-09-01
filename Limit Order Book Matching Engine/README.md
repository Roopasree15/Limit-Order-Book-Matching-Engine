# Limit Order Book & Matching Engine (C++)

A price-time priority order matching engine, modeled on how equity exchanges
match buy and sell orders. Built to demonstrate core C++ fundamentals: STL
containers, smart pointers, RAII, and algorithmic reasoning about order
matching — the same domain covered by SynapseWave's trading platform (order
management, risk, market data).

## What it does

- Maintains a live order book of resting BUY and SELL limit orders, keyed by
  price then arrival time (**price-time priority**, the standard exchange
  matching rule).
- Matches incoming orders against the best available opposite-side price:
  - **LIMIT** orders match up to their limit price, and rest on the book if
    not fully filled.
  - **MARKET** orders match immediately at whatever price is available
    ("sweep" the book), and never rest.
- Supports **partial fills** — an order can match against several resting
  orders across a price level.
- Supports **order cancellation** in O(log n) via an index of order id →
  location in the book.
- Prints a live bid/ask depth view after each event.

## Design notes

- `std::map<double, std::list<OrderPtr>>` represents each side of the book:
  the map keeps price levels sorted (best price first), and each level is a
  FIFO list of `shared_ptr<Order>` — giving correct price-time priority with
  O(log n) price-level lookup and O(1) insertion/removal within a level.
- An `unordered_map<orderId, OrderLocation>` lets `cancel()` find and erase
  any resting order directly, without scanning the book.
- `shared_ptr` is used deliberately (not raw pointers) so an order can be
  referenced simultaneously by a price-level list and the cancellation index
  without ownership ambiguity — a small, concrete RAII example.

## Build & run

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/OrderBook.cpp src/main.cpp -o orderbook_demo
./orderbook_demo
```

`main.cpp` runs a scripted demo: resting liquidity is added, an aggressive
limit order crosses the spread and partially fills across a price level, a
market order sweeps the remaining book, and a resting order is cancelled.

## Possible extensions (good next steps if you want to keep building this)

- Stop / stop-limit order types
- Order modification (cancel-replace) preserving or losing time priority
- A simple TCP/text protocol so orders can be submitted over a socket
- Multi-threading: one thread accepting orders, one matching, connected by
  a lock-free or mutex-protected queue
- Persisting the trade tape to a file or simple database

## Suggested resume bullet points

- Designed and implemented a C++ limit order matching engine supporting
  price-time priority, partial fills, market/limit order types, and O(log n)
  order cancellation, using STL containers (`map`, `list`, `unordered_map`)
  and RAII-managed smart pointers.
- Built a price-time priority matching algorithm handling multi-order fills
  across price levels, validated with a scripted simulation covering
  crossing orders, market sweeps, and cancellations.
