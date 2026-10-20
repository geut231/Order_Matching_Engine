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
void Order::show() const
{
    cout << "Order ID:" << id << endl;
    cout << "Price: " << rate << endl;
    cout << "Quantity: " << qty << endl;
    cout << "Original Quantity: " << originalqty << endl;
    cout << "Sequence: " << no << endl;
    cout << "Trader: " << trader << endl;
    cout << "Side: " << getsidestring() << endl;
    cout << "Status: " << getstatusstring() << endl;
}
void Order::showDetails()
{
    show();
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

int Order::getno() const
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
    if (q <= 0)
    {
        return;
    }
    qty = q;
    originalqty = q;
    status = Orderstatus::ACTIVE;
}

void Order::setrate(double r)
{
    if (r > 0)
        rate = r;
}

void Order::setstatus(Orderstatus s)
{
    status = s;
}

void Order::settrader(string t)
{
    if (!t.empty())
        trader = t;
}
void Order::setno(int n)
{
    if (n > 0)
    {
        no = n;
    }
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
    if (status == Orderstatus::CANCELLED || status == Orderstatus::FILLED || quantity <= 0 || quantity > qty)
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

string Order::getsidestring() const
{
    if (side == Orderside::BUY)
    {
        return "BUY";
    }

    return "SELL";
}

string Order::getstatusstring() const
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

BuyOrder::BuyOrder(int i, double r, int q, int n, string t)
    : Order(i, r, q, n, Orderside::BUY, t)
{
}

void BuyOrder::showDetails()
{
    cout << "\nBuy Order Details\n";
    show();
}

SellOrder::SellOrder(int i, double r, int q, int n, string t)
    : Order(i, r, q, n, Orderside::SELL, t)
{
}

void SellOrder::showDetails()
{
    cout << "\nSell Order Details\n";
    show();
}