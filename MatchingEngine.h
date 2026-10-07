#ifndef MATCHINGENGINE_H
#define MATCHINGENGINE_H

#include "Order.h"
#include "OrderBook.h"

using namespace std;

class MatchingEngine
{
private:
    OrderBook orderBook;

    int nextOrderId;
    int nextSequence;

public:
    MatchingEngine();

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
    void showTrades();
    void showOrderHistory();
    void showMarketSummary();
    void showDepth();

    int getNextOrderId();
    int getNextSequence();
};

#endif