class Order
{
protected:
    int id;
    double rate;
    int qty;
    int no;

public:
    Order(int i, double r, int q, int n);

    void show();

    int getid();
    double getrate();
    int getqty();
    int getno();

    void setqty(int q);
    void setrate(double r);

    bool valid();
};