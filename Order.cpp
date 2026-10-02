#include<iostream>
#include "Order.h"
using namespace std;

Order::Order(int i, double r, int q, int n)
{
    id=i;
    rate=r;
    qty=q;
    no=n;
}

void Order::show()
{
    cout<<"Order ID:" <<id <<endl;
    cout<<"Price: "<<rate <<endl;
    cout<<"Quantity: " <<qty<<endl;
    cout<<"Sequence: "<<no <<endl;
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

int Order::getno()
{
    return no;
}

void Order::setqty(int q)
{
    qty = q;
}

void Order::setrate(double r)
{
    rate = r;
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
    return true;
}