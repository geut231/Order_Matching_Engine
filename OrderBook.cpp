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