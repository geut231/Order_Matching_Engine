#include <iostream>
#include "Order.h"
using namespace std;

Order::Order(int i, double r, int q, int n, Orderside s, string t)
{
    id = i;
    rate = r;
    qty = q;
    originalqty = q;
    no = n;
    side = s;
    status = Orderstatus::ACTIVE;
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

    double Order::getrate() const
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
    bool Order::isBuy()
{
    return side == Orderside::BUY;
}

bool Order::isSell()
{
    return side == Orderside::SELL;
}

void Order::execute(int quantity)
{
    if (quantity <= 0 || quantity > qty)
    {
        return;
    }

    qty = qty - quantity;

    if (qty == 0)
    {
        status = Orderstatus::FILLED;
    }
    else
    {
        status = Orderstatus::PARTIALLY_FILLED;
    }
}

void Order::cancel()
{
    if (status != Orderstatus::FILLED)
    {
        status = Orderstatus::CANCELLED;
    }
}

string Order::getsidestring()
{
    if (side == Orderside::BUY)
    {
        return "BUY";
    }

    return "SELL";
}

string Order::getstatusstring()
{
    if (status == Orderstatus::ACTIVE)
    {
        return "ACTIVE";
    }
    else if (status == Orderstatus::PARTIALLY_FILLED)
    {
        return "PARTIALLY FILLED";
    }
    else if (status == Orderstatus::FILLED)
    {
        return "FILLED";
    }

    return "CANCELLED";
}