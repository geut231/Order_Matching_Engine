#ifndef MATCHINGENGINE_H
#define MATCHINGENGINE_H

#include "Order.h"
#include "OrderBook.h"

using namespace std;

class Database;
class MatchingEngine
{
private:
    OrderBook orderBook;

    int nextOrderId;
    int nextSequence;
    Database* db;

public:
    MatchingEngine(Database* database);

    bool placeBuyOrder(
        double price,
        int quantity,
        string trader
    );

    bool placeSellOrder(
        double price,
        int quantity,
        string trader
    );

    void processMatching();

    bool cancelOrder(int id);

    bool modifyOrder(
        int id,
        double newPrice,
        int newQuantity
    );

    bool searchOrder(
        int id,
        Order& result
    );

    void showOrderBook();

    int getNextOrderId();
    int getNextSequence();
};

#endif