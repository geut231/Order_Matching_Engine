#ifndef ORDERBOOK_H
#define ORDERBOOK_H
#include <iostream>
#include <queue>
#include <vector>
#include "Order.h"

using namespace std;

struct BuyCompare
{
    bool operator()(const Order &a, const Order &b) const
    {
        if (a.getrate() == b.getrate())
        {
            return a.getno() > b.getno();
        }

        return a.getrate() < b.getrate();
    }
};

struct SellCompare
{
    bool operator()(const Order &a, const Order &b) const
    {
        if (a.getrate() == b.getrate())
        {
            return a.getno() > b.getno();
        }

        return a.getrate() > b.getrate();
    }
};

class OrderBook
{
private:
    priority_queue<Order, vector<Order>, BuyCompare> buys;
    priority_queue<Order, vector<Order>, SellCompare> sells;
    vector<Order> cancelledHistory;

public:
    void addbuy(Order o);
    void addsell(Order o);
    void showbuy();
    void showsell();
    bool canmatch();
    void matchOrders();
    bool cancelOrder(int id);
    bool modifyOrder(int id, double newPrice, int newQuantity);
    bool modifyOrder(int id, double newPrice, int newQuantity, int newSequence);
    bool findOrder(int id, Order &result);
    void showOrderBook();
    void showCancelledHistory();
};
#endif