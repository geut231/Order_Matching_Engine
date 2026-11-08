CREATE DATABASE IF NOT EXISTS order_matching_engine;

USE order_matching_engine;

CREATE TABLE IF NOT EXISTS Orders (
    order_id INT PRIMARY KEY,
    rate DECIMAL(10,2),
    qty INT,
    original_qty INT,
    order_no INT,
    side VARCHAR(10),
    status VARCHAR(20),
    trader VARCHAR(100)
);

CREATE TABLE IF NOT EXISTS Trades (
    trade_id INT AUTO_INCREMENT PRIMARY KEY,
    buy_order_id INT,
    sell_order_id INT,
    trade_price DECIMAL(10,2),
    trade_quantity INT,
    trade_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);