#include "OrderBook.h"

void OrderBook::addbuy(Order o)
{
    buys.push(o);
}

void OrderBook::addsell(Order o)
{
    sells.push(o);
}

void OrderBook::showbuy()
{
    priority_queue<Order, vector<Order>, BuyCompare> temp = buys;

    cout<<"\nBUY ORDERS\n";

    while(!temp.empty())
    {
        temp.top().show();
        cout << endl;
        temp.pop();
    }
}

void OrderBook::showsell()
{
    priority_queue<Order, vector<Order>, SellCompare> temp = sells;

    cout<<"\nSELL ORDERS\n";

    while(!temp.empty())
    {
        temp.top().show();
        cout<<endl;
        temp.pop();
    }
}

bool OrderBook::canmatch()
{
    if(buys.empty() || sells.empty())
    {
        return false;
    }

    if(buys.top().getrate() >= sells.top().getrate())
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

        int quantity;

        if (buy.getqty() < sell.getqty())
        {
            quantity = buy.getqty();
        }
        else
        {
            quantity = sell.getqty();
        }

        buy.execute(quantity);
        sell.execute(quantity);

        buys.pop();
        sells.pop();

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

bool OrderBook::findOrder(int id, Order& result)
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
    if (newPrice <= 0 || newQuantity <= 0)
    {
        return false;
    }

    Order o(0, 0, 0, 0, Orderside::BUY);

    if (!findOrder(id, o))
    {
        return false;
    }

    if (o.getStatus() == Orderstatus::CANCELLED ||
        o.getStatus() == Orderstatus::FILLED)
    {
        return false;
    }

    cancelOrder(id);

    o.setrate(newPrice);
    o.setqty(newQuantity);
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