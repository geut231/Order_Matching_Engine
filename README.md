# Order Matching Engine

An electronic order matching engine that matches buy and sell limit orders using **Price-Time Priority**, backed by **MySQL** for persistence and a **Native C++ HTTP REST API** for live web dashboard interactions.

---

## Overview

An Order Matching Engine is a core component of modern electronic financial exchanges. Its primary responsibility is to match buy and sell orders according to predefined algorithmic rules.

The engine follows the **Price-Time Priority** principle:
1. **Price Priority**: The order with the better price gets higher priority (higher price for buys, lower price for sells).
2. **Time Priority**: When orders share the same price, the order submitted earlier gets higher priority.
3. **Execution**: A trade is executed whenever $\text{Highest Buy} \ge \text{Lowest Sell}$.

---

## Features

- **Limit Order Placement**: Buy and Sell limit orders with trader identification.
- **Price-Time Priority Matching**: Instant matching using max/min priority queues.
- **Full & Partial Execution**: Remaining quantities persist in the order book.
- **Order Cancellation & Modification**: Real-time order state updates.
- **MySQL Persistence**: Orders and executed trades are saved to disk with crash recovery on startup.
- **Native C++ HTTP REST API**: Integrated multi-threaded web server (`Winsock2`) exposing REST endpoints.
- **Real-Time Web Dashboard**: Live updating dashboard displaying order books, active orders, executed trade history, and market spreads.

---

## REST API Endpoints (Port 8080)

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/` or `/index.html` | Serves the web dashboard interface |
| `GET` | `/api/market-summary` | Returns Best Bid, Best Ask, Spread, Mid-price, and trade totals |
| `GET` | `/api/order-book` | Returns current bids and asks queues sorted by priority |
| `GET` | `/api/active-orders` | Returns all pending orders currently in the book |
| `GET` | `/api/trades` | Returns historical executed trades from MySQL |
| `POST` | `/api/orders` | Place a new Buy or Sell order and trigger matching |
| `POST` | `/api/cancel` | Cancel an active order by ID |
| `POST` | `/api/modify` | Modify price and quantity of an active order |

### Example Order Request (`POST /api/orders`)
```json
{
  "side": "BUY",
  "trader": "Alice",
  "price": 105.00,
  "quantity": 20
}
```

---

## Building and Running

### Build
Run `build.bat` or compile with MinGW / GCC:
```bash
g++ main.cpp Order.cpp OrderBook.cpp MatchingEngine.cpp Database.cpp HttpServer.cpp -I"C:\msys64\ucrt64\include\mysql" -L"C:\msys64\ucrt64\lib" -lmysqlclient -lws2_32 -o engine.exe
```

### Run
```bash
./engine.exe
```
Once started:
- The **interactive console menu** runs in the terminal.
- The **HTTP REST API & Web Dashboard** is live at: `http://localhost:8080`.
