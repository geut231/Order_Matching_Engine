#include <iostream>
#include <string>
using namespace std;
class Order
{
protected:
    int orderid;
    double price;
    int quantity;
    int seq;

public:
    Order(int id, double p, int q, int s)
    {
        orderid = id;
        price = p;
        quantity = q;
        seq = s;
    }
    void display()
    {
        cout << "Order ID: " << orderid << endl;
        cout << "Price: " << price << endl;
        cout << "Quantity: " << quantity << endl;
        cout << "Sequence: " << seq << endl;
    }
    int getOrderId()
    {
        return orderid;
    }

    double getPrice()
    {
        return price;
    }

    int getQuantity()
    {
        return quantity;
    }

    int getSequence()
    {
        return seq;
    }
};

class BuyOrder : public Order
{
public:
    BuyOrder(int id, double p, int q, int s)
        : Order(id, p, q, s)
    {
    }
};

class SellOrder : public Order
{
public:
    SellOrder(int id, double p, int q, int s)
        : Order(id, p, q, s)
    {
    }
};