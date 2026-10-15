#include <iostream>
#include <queue>
#include <vector>
#include "Order.h"

using namespace std;

struct BuyCompare
{
    bool operator()(Order a, Order b)
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
    bool operator()(Order a, Order b)
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

public:
    void addbuy(Order o);
    void addsell(Order o);
    void showbuy();
    void showsell();
    bool canmatch();
    void matchOrders();
    bool cancelOrder(int id);
    bool modifyOrder(int id, double newPrice, int newQuantity);
    bool findOrder(int id, Order& result);
    void showOrdersBook();
};