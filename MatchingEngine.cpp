#include "Database.h"
#include "MatchingEngine.h"

using namespace std;

MatchingEngine::MatchingEngine(Database* database)
{
    db = database;
    orderBook.setDatabase(database);
    db->loadOrders(orderBook);
    nextOrderId = db->getNextOrderId();
    nextSequence = db->getNextSequence();
}

bool MatchingEngine::placeBuyOrder(
    double price,
    int quantity,
    string trader)
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
        trader);

    if (!order.valid())
    {
        return false;
    }

        if (!db->insertOrder(
            order.getid(),
            order.getrate(),
            order.getqty(),
            order.getoriginalqty(),
            order.getno(),
            order.getsidestring(),
            order.getstatusstring(),
            order.getTrader()))
    {
        return false;
    }

    orderBook.addbuy(order);
    nextOrderId++;
    nextSequence++;

    processMatching();

    return true;
}

bool MatchingEngine::placeSellOrder(
    double price,
    int quantity,
    string trader)
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
        trader);

    if (!order.valid())
    {
        return false;
    }
    
    if (!db->insertOrder(
            order.getid(),
            order.getrate(),
            order.getqty(),
            order.getoriginalqty(),
            order.getno(),
            order.getsidestring(),
            order.getstatusstring(),
            order.getTrader()))
    {
        return false;
    }
    orderBook.addsell(order);

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
     Order result(1, 1.0, 1, 1, Orderside::BUY);

    if (!orderBook.findOrder(id, result))
        return false;

    if (!db->updateOrderStatus(id, "CANCELLED"))
        return false;
    return orderBook.cancelOrder(id);
}

bool MatchingEngine::modifyOrder(
    int id,
    double newPrice,
    int newQuantity)
{
    if (newPrice <= 0 || newQuantity <= 0)
    {
        return false;
    }

    Order result(1, 1.0, 1, 1, Orderside::BUY);

    if (!orderBook.findOrder(id, result))
        return false;

    if (!db->updateOrder(
            id,
            newPrice,
            newQuantity,
            nextSequence,
            "ACTIVE"))
        return false;

    bool modified = orderBook.modifyOrder(
        id,
        newPrice,
        newQuantity,
        nextSequence);

    if (modified)
    {
        nextSequence++;
        processMatching();
    }

    return modified;
}
bool MatchingEngine::searchOrder(
    int id,
    Order &result)
{
    return orderBook.findOrder(id, result);
}

void MatchingEngine::showOrderBook()
{
    orderBook.showOrderBook();
}

int MatchingEngine::getNextOrderId()
{
    return nextOrderId;
}

int MatchingEngine::getNextSequence()
{
    return nextSequence;
}