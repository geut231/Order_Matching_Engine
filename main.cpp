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
    order(int id, double p, int q, int s) 
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