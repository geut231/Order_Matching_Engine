#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
#include <string>
#include <vector>
using namespace std;

struct TradeRecord
{
    int tradeId;
    int buyOrderId;
    int sellOrderId;
    double price;
    int quantity;
    string time;
};

class OrderBook;
class Database
{
private:
    MYSQL *conn;

public:
    Database();
    ~Database();
    int getNextOrderId();
    int getNextSequence();
    bool connect();
    bool insertOrder(
    int id,
    double price,
    int quantity,
    int originalQuantity,
    int orderNo,
    string side,
    string status,
    string trader
);
    bool updateOrderStatus(int id, string status);
    bool updateOrder(
    int id,
    double price,
    int quantity,
    int orderNo,
    string status
);
    bool insertTrade(
    int buyOrderId,
    int sellOrderId,
    double tradePrice,
    int tradeQuantity
);
    bool loadOrders(OrderBook& orderBook);
    void showTradeHistory();
    vector<TradeRecord> getTradeHistory();
    void close();
};

#endif