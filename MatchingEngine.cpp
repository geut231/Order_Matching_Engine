#include "MatchingEngine.h"

using namespace std;

MatchingEngine::MatchingEngine()
{
    nextOrderId = 1;
    nextSequence = 1;
}

bool MatchingEngine::placeBuyOrder(
    double price,
    int quantity,
    string trader
)
{
    if (price <= 0 || quantity <= 0 || trader.empty())
    {
        return false;
    }

    Order order(
        nextOrderId,
        price,
        quantity,
        nextSequence,
        Orderside::BUY,
        trader
    );

    if (!order.valid())
    {
        return false;
    }

    if (!orderBook.addbuy(order))
    {
        return false;
    }

    nextOrderId++;
    nextSequence++;

    processMatching();

    return true;
}

bool MatchingEngine::placeSellOrder(
    double price,
    int quantity,
    string trader
)
{
    if (price <= 0 || quantity <= 0 || trader.empty())
    {
        return false;
    }

    Order order(
        nextOrderId,
        price,
        quantity,
        nextSequence,
        Orderside::SELL,
        trader
    );

    if (!order.valid())
    {
        return false;
    }

    if (!orderBook.addsell(order))
    {
        return false;
    }

    nextOrderId++;
    nextSequence++;

    processMatching();

    return true;
}

void MatchingEngine::processMatching()
{
    orderBook.matchOrders();
}

bool MatchingEngine::cancelOrder(int id)
{
    return orderBook.cancelOrder(id);
}

bool MatchingEngine::modifyOrder(
    int id,
    double newPrice,
    int newQuantity
)
{
    if (newPrice <= 0 || newQuantity <= 0)
    {
        return false;
    }

    bool modified = orderBook.modifyOrder(
        id,
        newPrice,
        newQuantity
    );

    if (modified)
    {
        processMatching();
    }

    return modified;
}

bool MatchingEngine::searchOrder(
    int id,
    Order& result
)
{
    return orderBook.findOrder(id, result);
}

void MatchingEngine::showOrderBook()
{
    orderBook.showOrderBook();
}

void MatchingEngine::showTrades()
{
    orderBook.showTrades();
}

void MatchingEngine::showOrderHistory()
{
    orderBook.showOrderHistory();
}

void MatchingEngine::showMarketSummary()
{
    orderBook.showMarketSummary();
}

void MatchingEngine::showDepth()
{
    orderBook.showDepth();
}
int MatchingEngine::getNextOrderId()
{
    return nextOrderId;
}

int MatchingEngine::getNextSequence()
{
    return nextSequence;
}