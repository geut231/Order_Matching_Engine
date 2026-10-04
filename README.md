# Order_Matching_Engine
An order matching engine that matches buy and sell limit orders using price-time priority.
## Overview

An Order Matching Engine is a core component of electronic trading platforms. Its primary responsibility is to match buy and sell orders according to predefined rules.

This project implements a simplified exchange-style matching engine using Data Structures and Object-Oriented Programming concepts in C++.

The engine follows the **Price-Time Priority** principle:

1. The order with the better price gets higher priority.
2. If multiple orders have the same price, the order submitted earlier gets higher priority.
3. A trade is executed when the highest buy price is greater than or equal to the lowest sell price.
## Features

- Buy order placement
- Sell order placement
- Price-Time Priority matching
- Automatic order matching
- Full order execution
- Partial order execution
- Order cancellation
- Order modification
- Order search by Order ID
- Active order tracking
- Trade history
- Market summary
- Best Bid and Best Ask
- Bid-Ask Spread calculation
- Mid-market price calculation
- Order book depth
- Trader identification
- Input validation