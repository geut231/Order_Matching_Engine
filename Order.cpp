#include <iostream>
#include "Order.h"
using namespace std;

Order::Order(int i, double r, int q, int n, Orderside s, string t)
{
    id = i;
    rate = r;
    qty = q;
    no = n;
    side = s;
    status = Orderatatus::ACTIVE;
    trader = t;
}

void Order::show()
{
    cout << "Order ID:" << id << endl;
    cout << "Price: " << rate << endl;
    cout << "Quantity: " << qty << endl;
    cout << "Sequence: " << no << endl;
}

int Order::getid()
{
    return id;
}

double Order::getrate()
{
    return rate;
}

int Order::getqty()
{
    return qty;
}

int Order::getoriginalqty()
{
    return originalqty;
}

int Order::getno()
{
    return no;
}

Orderside Order::getSide()
{
    return side;
}

Orderstatus Order::getStatus()
{
    return status;
}

string Order::getTrader()
{
    return trader;
}

void Order::setqty(int q)
{
    qty = q;

    if (qty <= 0)
    {
        qty = 0;
        status = Orderstatus::FILLED;
    }
    else if (qty < originalqty)
    {
        status = Orderstatus::PARTIALLY_FILLED;
    }
    else
    {
        status = Orderstatus::ACTIVE;
    }
}

void Order::setrate(double r)
{
    rate = r;
}

void Order::setstatus(Orderstatus s)
{
    status = s;
}

void Order::settrader(string t)
{
    trader = t;
}

bool Order::valid()
{
    if (id <= 0)
    {
        return false;
    }

    if (rate <= 0)
    {
        return false;
    }

    if (qty <= 0)
    {
        return false;
    }

    if (no <= 0)
    {
        return false;
    }

    if (trader.empty())
    {
        return false;
    }
    return true;
}