# Limit Order Book & Matching Engine

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)

A price-time priority order matching engine in modern C++ — the same core
mechanism real exchanges and trading platforms use to match buy and sell
orders. Built to demonstrate STL proficiency, RAII/smart-pointer discipline,
and algorithmic reasoning about order matching.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Demo Output](#demo-output)
- [Design](#design)
- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [Possible Extensions](#possible-extensions)
- [License](#license)

## Overview

An order matching engine sits at the heart of every trading system: it takes
incoming buy/sell orders and matches them against a live book of resting
orders, following price-time priority — best price first, and among orders
at the same price, first-come-first-served.

This project implements that core loop from scratch in C++17, with a small
scripted simulation showing it handle crossing orders, partial fills, market
sweeps, and cancellations.

## Features

- 📈 **Price-time priority matching** — orders match at the best available
  price; ties broken strictly by arrival order.
- ⚡ **LIMIT and MARKET order types** — limit orders rest on the book if
  unfilled; market orders sweep immediately and never rest.
- 🔀 **Partial fills across multiple price levels** — a single incoming order
  can match against several resting orders in sequence.
- ❌ **O(log n) order cancellation** via a direct order-id → book-location
  index.
- 🖥️ **Live book depth printout** after every event.

## Demo Output

```
> Aggressive BUY LIMIT 101.60 x 120 arrives (crosses the spread)
  TRADE  id=1  buy#6 x sell#1  100 @ 101.50
  TRADE  id=2  buy#6 x sell#3  20 @ 101.50

--- Order Book: INFY ---
BID QTY     BID PRICE    | ASK PRICE   ASK QTY
150         100.90       | 101.50      30
300         100.75       | 101.75      200
```

## Design

| Component | Structure | Why |
|---|---|---|
| Each side of the book | `std::map<double, std::list<OrderPtr>>` | Map keeps price levels sorted (best price first, O(log n) access); each level is a FIFO list for time priority |
| Order lookup for cancellation | `std::unordered_map<uint64_t, OrderLocation>` | O(1) average lookup from order id straight to its position in the book |
| Order ownership | `std::shared_ptr<Order>` | An order is referenced by both a price-level list and the cancellation index at once — shared ownership avoids dangling pointers without manual memory management |

## Getting Started

### Prerequisites
- A C++17 compiler (g++ 9+ or clang++ 10+)

### Build & Run

```bash
git clone https://github.com/Roopasree15/orderbook-engine.git
cd orderbook-engine
g++ -std=c++17 -Wall -Wextra -Iinclude src/OrderBook.cpp src/main.cpp -o orderbook_demo
./orderbook_demo
```

`main.cpp` runs a scripted walkthrough: resting liquidity is added, an
aggressive limit order crosses the spread and partially fills across a price
level, a market order sweeps the remaining book, and a resting order is
placed then cancelled.

## Project Structure

```
orderbook-engine/
├── include/
│   ├── Order.h        # Order & Trade data structures
│   └── OrderBook.h    # OrderBook class interface
├── src/
│   ├── OrderBook.cpp  # Matching engine implementation
│   └── main.cpp       # Scripted demo / simulation
└── README.md
```

## Possible Extensions

- [ ] Stop / stop-limit order types
- [ ] Order modification (cancel-replace), with correct time-priority rules
- [ ] A simple TCP/text protocol for submitting orders over a socket
- [ ] Multi-threading — separate accept and matching threads connected by a
      thread-safe queue
- [ ] Persist the trade tape to a file or lightweight database
- [ ] Unit tests (Catch2 / GoogleTest) covering edge cases: self-crossing
      orders, zero-quantity orders, tie-breaking

## License

MIT — free to use, modify, and learn from.
