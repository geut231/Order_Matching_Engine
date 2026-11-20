#include "Database.h"
#include "OrderBook.h"
#include "TraderHistory.h"
#include <algorithm>
#include <iostream>

using namespace std;

OrderBook::OrderBook() : db(nullptr), history(nullptr)
{
}

void OrderBook::setDatabase(Database* database)
{
    db = database;
}

void OrderBook::setHistory(TraderHistory* h)
{
    history = h;
}

void OrderBook::addbuy(Order o)
{
    if (o.valid() && o.isBuy())
    {
        double price = o.getrate();
        buyBook[price].push_back(o);
        auto it = std::prev(buyBook[price].end());
        orderLookup[o.getid()] = { Orderside::BUY, price, it };
    }
}

void OrderBook::addsell(Order o)
{
    if (o.valid() && o.isSell())
    {
        double price = o.getrate();
        sellBook[price].push_back(o);
        auto it = std::prev(sellBook[price].end());
        orderLookup[o.getid()] = { Orderside::SELL, price, it };
    }
}

void OrderBook::showbuy()
{
    cout << "\nBUY ORDERS\n";
    if (buyBook.empty())
    {
        cout << "No buy orders.\n";
        return;
    }

    for (const auto& pair : buyBook)
    {
        for (const auto& o : pair.second)
        {
            o.show();
            cout << endl;
        }
    }
}

void OrderBook::showsell()
{
    cout << "\nSELL ORDERS\n";
    if (sellBook.empty())
    {
        cout << "No sell orders.\n";
        return;
    }

    for (const auto& pair : sellBook)
    {
        for (const auto& o : pair.second)
        {
            o.show();
            cout << endl;
        }
    }
}

bool OrderBook::canmatch()
{
    if (buyBook.empty() || sellBook.empty())
    {
        return false;
    }

    // Top buy price >= Top sell price
    return buyBook.begin()->first >= sellBook.begin()->first;
}

void OrderBook::showOrderBook()
{
    showbuy();
    showsell();
}

void OrderBook::matchOrders()
{
    while (canmatch())
    {
        auto buyPriceIt = buyBook.begin();
        auto sellPriceIt = sellBook.begin();

        Order& buy = buyPriceIt->second.front();
        Order& sell = sellPriceIt->second.front();

        int quantity = min(buy.getqty(), sell.getqty());
        double tradePrice;

        if (buy.getno() < sell.getno())
            tradePrice = buy.getrate();
        else
            tradePrice = sell.getrate();

        buy.execute(quantity);
        sell.execute(quantity);

        if (history)
        {
            history->logTradeExecution(buy.getTrader(), "BUY", buy.getid(), tradePrice, quantity, sell.getTrader(), sell.getid());
            history->logTradeExecution(sell.getTrader(), "SELL", sell.getid(), tradePrice, quantity, buy.getTrader(), buy.getid());
        }

        if (db)
        {
            if (!db->insertTrade(buy.getid(), sell.getid(), tradePrice, quantity))
            {
                cout << "Trade could not be saved to DB.\n";
            }

            db->updateOrder(buy.getid(), buy.getrate(), buy.getqty(),
                            buy.getno(), buy.getstatusstring());

            db->updateOrder(sell.getid(), sell.getrate(), sell.getqty(),
                            sell.getno(), sell.getstatusstring());
        }

        cout << "\nTrade Executed\n";
        cout << "Buy Order ID: " << buy.getid() << endl;
        cout << "Sell Order ID: " << sell.getid() << endl;
        cout << "Quantity: " << quantity << endl;
        cout << "Trade Price: " << tradePrice << endl;

        // Efficient removal of filled orders
        if (buy.getqty() == 0)
        {
            orderLookup.erase(buy.getid());
            buyPriceIt->second.pop_front();
            if (buyPriceIt->second.empty())
            {
                buyBook.erase(buyPriceIt);
            }
        }

        if (sell.getqty() == 0)
        {
            orderLookup.erase(sell.getid());
            sellPriceIt->second.pop_front();
            if (sellPriceIt->second.empty())
            {
                sellBook.erase(sellPriceIt);
            }
        }
    }
}

// O(1) Order Lookup via hash map
bool OrderBook::findOrder(int id, Order &result)
{
    auto it = orderLookup.find(id);
    if (it == orderLookup.end())
    {
        return false;
    }

    result = *(it->second.it);
    return true;
}

// O(1) Order Cancellation via iterator erasure
bool OrderBook::cancelOrder(int id)
{
    auto lookupIt = orderLookup.find(id);
    if (lookupIt == orderLookup.end())
    {
        return false;
    }

    Orderside side = lookupIt->second.side;
    double price = lookupIt->second.price;
    auto listIt = lookupIt->second.it;

    Order o = *listIt;
    o.cancel();
    cancelledHistory.push_back(o);

    if (side == Orderside::BUY)
    {
        buyBook[price].erase(listIt);
        if (buyBook[price].empty())
        {
            buyBook.erase(price);
        }
    }
    else
    {
        sellBook[price].erase(listIt);
        if (sellBook[price].empty())
        {
            sellBook.erase(price);
        }
    }

    orderLookup.erase(lookupIt);

    if (history)
    {
        history->logOrderCancellation(o.getTrader(), o.getid(), o.getsidestring(), o.getrate(), o.getqty());
    }

    return true;
}

bool OrderBook::modifyOrder(int id, double newPrice, int newQuantity)
{
    Order o(1, 1.0, 1, 1, Orderside::BUY);
    if (!findOrder(id, o))
    {
        return false;
    }

    int newSequence = o.getno() + 1;
    return modifyOrder(id, newPrice, newQuantity, newSequence);
}

// O(1) erase + O(log P) re-insertion
bool OrderBook::modifyOrder(
    int id,
    double newPrice,
    int newQuantity,
    int newSequence)
{
    if (newPrice <= 0 || newQuantity <= 0 || newSequence <= 0)
    {
        return false;
    }

    auto lookupIt = orderLookup.find(id);
    if (lookupIt == orderLookup.end())
    {
        return false;
    }

    Orderside side = lookupIt->second.side;
    double oldPrice = lookupIt->second.price;
    auto listIt = lookupIt->second.it;

    Order o = *listIt;
    if (o.getStatus() == Orderstatus::CANCELLED || o.getStatus() == Orderstatus::FILLED)
    {
        return false;
    }

    // 1. Erase from old price level in O(1)
    if (side == Orderside::BUY)
    {
        buyBook[oldPrice].erase(listIt);
        if (buyBook[oldPrice].empty())
        {
            buyBook.erase(oldPrice);
        }
    }
    else
    {
        sellBook[oldPrice].erase(listIt);
        if (sellBook[oldPrice].empty())
        {
            sellBook.erase(oldPrice);
        }
    }

    orderLookup.erase(lookupIt);

    // 2. Update order attributes
    o.setrate(newPrice);
    o.setqty(newQuantity);
    o.setno(newSequence);
    o.setstatus(Orderstatus::ACTIVE);

    // 3. Re-insert at new price level in O(log P)
    if (side == Orderside::BUY)
    {
        addbuy(o);
    }
    else
    {
        addsell(o);
    }

    if (history)
    {
        history->logOrderModification(o.getTrader(), id, newPrice, newQuantity);
    }

    return true;
}

void OrderBook::showCancelledHistory()
{
    cout << "\nCANCELLED ORDER HISTORY\n";

    if (cancelledHistory.empty())
    {
        cout << "No cancelled orders in this session.\n";
        return;
    }

    for (const Order &o : cancelledHistory)
    {
        o.show();
        cout << '\n';
    }
}

vector<Order> OrderBook::getBuys() const
{
    vector<Order> list;
    for (const auto& pair : buyBook)
    {
        for (const auto& order : pair.second)
        {
            list.push_back(order);
        }
    }
    return list;
}

vector<Order> OrderBook::getSells() const
{
    vector<Order> list;
    for (const auto& pair : sellBook)
    {
        for (const auto& order : pair.second)
        {
            list.push_back(order);
        }
    }
    return list;
}

double OrderBook::getBestBid() const
{
    if (buyBook.empty()) return 0.0;
    return buyBook.begin()->first;
}

double OrderBook::getBestAsk() const
{
    if (sellBook.empty()) return 0.0;
    return sellBook.begin()->first;
}

double OrderBook::getSpread() const
{
    if (buyBook.empty() || sellBook.empty()) return 0.0;
    double s = sellBook.begin()->first - buyBook.begin()->first;
    return s > 0 ? s : 0.0;
}

double OrderBook::getMidPrice() const
{
    if (buyBook.empty() && sellBook.empty()) return 0.0;
    if (buyBook.empty()) return sellBook.begin()->first;
    if (sellBook.empty()) return buyBook.begin()->first;
    return (buyBook.begin()->first + sellBook.begin()->first) / 2.0;
}
