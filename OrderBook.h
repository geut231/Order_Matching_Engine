#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>
#include "Order.h"

using namespace std;

class Database;
class TraderHistory;

// Location structure for O(1) order index
struct OrderLocation
{
    Orderside side;
    double price;
    std::list<Order>::iterator it;
};

class OrderBook
{
private:
    Database* db;
    TraderHistory* history;

    // L2 / L3 Order Book: Price -> FIFO list of orders
    // Bids sorted Descending (highest price first)
    std::map<double, std::list<Order>, std::greater<double>> buyBook;
    // Asks sorted Ascending (lowest price first)
    std::map<double, std::list<Order>, std::less<double>> sellBook;

    // Hash index: order_id -> OrderLocation for O(1) lookup, cancel, and modify
    std::unordered_map<int, OrderLocation> orderLookup;

    std::vector<Order> cancelledHistory;

public:
    OrderBook();

    void setDatabase(Database* database);
    void setHistory(TraderHistory* h);

    void addbuy(Order o);
    void addsell(Order o);

    void showbuy();
    void showsell();
    void showOrderBook();
    void showCancelledHistory();

    bool canmatch();
    void matchOrders();

    // High performance O(1) / O(log P) operations
    bool cancelOrder(int id);
    bool modifyOrder(int id, double newPrice, int newQuantity);
    bool modifyOrder(int id, double newPrice, int newQuantity, int newSequence);
    bool findOrder(int id, Order &result);

    // Getters for API and statistics
    vector<Order> getBuys() const;
    vector<Order> getSells() const;
    double getBestBid() const;
    double getBestAsk() const;
    double getSpread() const;
    double getMidPrice() const;
};

#endif
