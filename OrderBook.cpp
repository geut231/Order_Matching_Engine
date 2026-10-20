#include "OrderBook.h"
#include <algorithm>
void OrderBook::addbuy(Order o)
{
    if (o.valid() && o.isBuy())
        buys.push(o);
}

void OrderBook::addsell(Order o)
{
    if (o.valid() && o.isSell())
        sells.push(o);
}

void OrderBook::showbuy()
{
    priority_queue<Order, vector<Order>, BuyCompare> temp = buys;

    cout << "\nBUY ORDERS\n";
    if (temp.empty())
    {
        cout << "No buy orders.\n";
    }

    while (!temp.empty())
    {
        temp.top().show();
        cout << endl;
        temp.pop();
    }
}

void OrderBook::showsell()
{
    priority_queue<Order, vector<Order>, SellCompare> temp = sells;

    cout << "\nSELL ORDERS\n";

    if (temp.empty())
    {
        cout << "No sell orders.\n";
    }

    while (!temp.empty())
    {
        temp.top().show();
        cout << endl;
        temp.pop();
    }
}

bool OrderBook::canmatch()
{
    if (buys.empty() || sells.empty())
    {
        return false;
    }

    if (buys.top().getrate() >= sells.top().getrate())
    {
        return true;
    }

    return false;
}
void OrderBook::showOrderBook()
{
    showbuy();
    showsell();
}

void OrderBook::matchOrders()
{
    while (canmatch())
    {
        Order buy = buys.top();
        Order sell = sells.top();

        buys.pop();
        sells.pop();

        int quantity = min(buy.getqty(), sell.getqty());

        double tradePrice;

        if (buy.getno() < sell.getno())
        {
            tradePrice = buy.getrate();
        }
        else
        {
            tradePrice = sell.getrate();
        }

        buy.execute(quantity);
        sell.execute(quantity);

        if (buy.getqty() > 0)
        {
            buys.push(buy);
        }

        if (sell.getqty() > 0)
        {
            sells.push(sell);
        }

        cout << "\nTrade Executed\n";
        cout << "Buy Order ID: " << buy.getid() << endl;
        cout << "Sell Order ID: " << sell.getid() << endl;
        cout << "Quantity: " << quantity << endl;
        cout << "Trade Price: " << sell.getrate() << endl;
    }
}

bool OrderBook::cancelOrder(int id)
{
    priority_queue<Order, vector<Order>, BuyCompare> tempBuy;
    priority_queue<Order, vector<Order>, SellCompare> tempSell;

    bool found = false;

    while (!buys.empty())
    {
        Order o = buys.top();
        buys.pop();

        if (o.getid() == id)
        {
            o.cancel();
            cancelledHistory.push_back(o);
            found = true;
        }
        else
        {
            tempBuy.push(o);
        }
    }

    while (!sells.empty())
    {
        Order o = sells.top();
        sells.pop();

        if (o.getid() == id)
        {
            o.cancel();
            cancelledHistory.push_back(o);
            found = true;
        }
        else
        {
            tempSell.push(o);
        }
    }

    buys = tempBuy;
    sells = tempSell;

    return found;
}

bool OrderBook::findOrder(int id, Order &result)
{
    priority_queue<Order, vector<Order>, BuyCompare> tempBuy;
    priority_queue<Order, vector<Order>, SellCompare> tempSell;

    bool found = false;

    while (!buys.empty())
    {
        Order o = buys.top();
        buys.pop();

        if (o.getid() == id)
        {
            result = o;
            found = true;
        }

        tempBuy.push(o);
    }

    while (!sells.empty())
    {
        Order o = sells.top();
        sells.pop();

        if (o.getid() == id)
        {
            result = o;
            found = true;
        }

        tempSell.push(o);
    }

    buys = tempBuy;
    sells = tempSell;

    return found;
}

bool OrderBook::modifyOrder(int id, double newPrice, int newQuantity)
{
    Order o(1, 1.0, 1, 1, Orderside::BUY);

    if (!findOrder(id, o))
    {
        return false;
    }

    int newSequence = o.getno() + 1;

    return modifyOrder(id, newPrice, newQuantity, newSequence);
}
bool OrderBook::modifyOrder(
    int id,
    double newPrice,
    int newQuantity,
    int newSequence)
{
    if (newPrice <= 0 || newQuantity <= 0 || newSequence <= 0)
    {
        return false;
    }

    Order o(1, 1.0, 1, 1, Orderside::BUY);

    if (!findOrder(id, o))
    {
        return false;
    }

    if (o.getStatus() == Orderstatus::CANCELLED ||
        o.getStatus() == Orderstatus::FILLED)
    {
        return false;
    }

    priority_queue<Order, vector<Order>, BuyCompare> tempBuy;
    priority_queue<Order, vector<Order>, SellCompare> tempSell;

    while (!buys.empty())
    {
        Order current = buys.top();
        buys.pop();

        if (current.getid() != id)
        {
            tempBuy.push(current);
        }
    }

    while (!sells.empty())
    {
        Order current = sells.top();
        sells.pop();

        if (current.getid() != id)
        {
            tempSell.push(current);
        }
    }

    buys = tempBuy;
    sells = tempSell;

    o.setrate(newPrice);
    o.setqty(newQuantity);
    o.setno(newSequence);
    o.setstatus(Orderstatus::ACTIVE);

    if (o.isBuy())
    {
        addbuy(o);
    }
    else
    {
        addsell(o);
    }

    return true;
}

void OrderBook::showCancelledHistory()
{
    cout << "\nCANCELLED ORDER HISTORY\n";

    if (cancelledHistory.empty())
    {
        cout << "No cancelled orders in this session.\n";
        return;
    }

    for (const Order &o : cancelledHistory)
    {
        o.show();
        cout << '\n';
    }
}