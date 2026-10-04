#ifndef ORDER_H
#define ORDER_H

#include <iostream>
#include <string>
using namespace std;

enum class Orderside
{
    BUY,
    SELL
};

enum class Orderstatus
{
    ACTIVE,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED
};

class Order
{
protected:
    int id;
    double rate;
    int qty;
    int originalqty;
    int no;
    Orderside side;
    Orderstatus status;
    string trader;

public:
    Order(int i, double r, int q, int n, Orderside s, string t = "Anonymous");

    void show();

    int getid();
    double getrate();
    int getqty();
    int getoriginalqty();
    int getno();
    Orderside getSide();
    Orderstatus getStatus();
    string getTrader();

    void setqty(int q);
    void setrate(double r);
    void setstatus(Orderstatus s);
    void settrader(string t);

    bool valid();
    bool isBuy();
    bool isSell();

    void execute(int quantity);
    void cancel();

    string getsidestring();
    string getstatusstring();
};

#endif
