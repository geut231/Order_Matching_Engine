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
    virtual ~Order() = default;

    virtual void showDetails();

    void show() const;

    int getid();
    double getrate() const;
    int getqty();
    int getoriginalqty();
    int getno() const;
    Orderside getSide();
    Orderstatus getStatus();
    string getTrader();

    void setqty(int q);
    void setrate(double r);
    void setstatus(Orderstatus s);
    void settrader(string t);
    void setno(int n);

    bool valid();
    bool isBuy();
    bool isSell();

    void execute(int quantity);
    void cancel();

    string getsidestring() const;
    string getstatusstring() const;
};

class BuyOrder : public Order
{
public:
    BuyOrder(int i, double r, int q, int n,
             string t = "Anonymous");

    void showDetails() override;
};

class SellOrder : public Order
{
public:
    SellOrder(int i, double r, int q, int n,
              string t = "Anonymous");

    void showDetails() override;
};

#endif
