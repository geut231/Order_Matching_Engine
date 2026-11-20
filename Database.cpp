#include <string>
#include "Database.h"
#include "OrderBook.h"
#include "db_config.h"
#include <iostream>
#include <algorithm>
using namespace std;

Database::Database()
{
    conn = mysql_init(NULL);
    connected = false;
    nextFallbackTradeId = 1;
}

Database::~Database()
{
    close();
}

bool Database::connect()
{
    if (conn && mysql_real_connect(
            conn,
            DB_HOST,
            DB_USER,
            DB_PASSWORD,
            DB_NAME,
            3306,
            NULL,
            0))
    {
        cout << "[Database] MySQL Connected Successfully!\n";
        connected = true;
        return true;
    }

    connected = false;
    cout << "\n[Database] Notice: Could not connect to MySQL ("
         << (conn ? mysql_error(conn) : "Init error") << ").\n";
    cout << "[Database] Running in In-Memory / Standalone Mode.\n";
    cout << "           (All matching, API endpoints, and web dashboard will work in RAM)\n\n";

    return false;
}

int Database::getNextOrderId()
{
    if (!connected || !conn) return 1;

    string query = "SELECT MAX(order_id) FROM Orders";

    if (mysql_query(conn, query.c_str()) == 0)
    {
        MYSQL_RES *result = mysql_store_result(conn);
        if (!result) return 1;
        MYSQL_ROW row = mysql_fetch_row(result);

        int id = (row == NULL || row[0] == NULL) ? 1 : stoi(row[0]) + 1;

        mysql_free_result(result);

        return id;
    }

    return 1;
}

int Database::getNextSequence()
{
    if (!connected || !conn) return 1;

    string query = "SELECT MAX(order_no) FROM Orders";

    if (mysql_query(conn, query.c_str()) == 0)
    {
        MYSQL_RES *result = mysql_store_result(conn);
        if (!result) return 1;
        MYSQL_ROW row = mysql_fetch_row(result);

        int sequence = (row == NULL || row[0] == NULL) ? 1 : stoi(row[0]) + 1;

        mysql_free_result(result);

        return sequence;
    }

    return 1;
}

bool Database::insertOrder(
    int id,
    double price,
    int quantity,
    int originalQuantity,
    int orderNo,
    string side,
    string status,
    string trader)
{
    if (!connected || !conn)
    {
        return true;
    }

    string query =
        "INSERT INTO Orders VALUES (" +
        to_string(id) + ", " +
        to_string(price) + ", " +
        to_string(quantity) + ", " +
        to_string(originalQuantity) + ", " +
        to_string(orderNo) + ", '" +
        side + "', '" +
        status + "', '" +
        trader + "')";

    if (mysql_query(conn, query.c_str()) == 0)
    {
        return true;
    }

    cout << "Order could not be saved to DB: "
         << mysql_error(conn) << endl;

    return false;
}

bool Database::updateOrderStatus(int id, string status)
{
    if (!connected || !conn)
    {
        return true;
    }

    string query =
        "UPDATE Orders SET status = '" + status +
        "' WHERE order_id = " + to_string(id);

    if (mysql_query(conn, query.c_str()) == 0)
    {
        return true;
    }

    cout << "Order status update failed: "
         << mysql_error(conn) << endl;

    return false;
}

bool Database::updateOrder(
    int id,
    double price,
    int quantity,
    int orderNo,
    string status)
{
    if (!connected || !conn)
    {
        return true;
    }

    string query =
        "UPDATE Orders SET rate = " + to_string(price) +
        ", qty = " + to_string(quantity) +
        ", order_no = " + to_string(orderNo) +
        ", status = '" + status +
        "' WHERE order_id = " + to_string(id);

    if (mysql_query(conn, query.c_str()) == 0)
    {
        return true;
    }

    cout << "Order update failed: "
         << mysql_error(conn) << endl;

    return false;
}

bool Database::insertTrade(
    int buyOrderId,
    int sellOrderId,
    double tradePrice,
    int tradeQuantity)
{
    TradeRecord tr;
    tr.tradeId = nextFallbackTradeId++;
    tr.buyOrderId = buyOrderId;
    tr.sellOrderId = sellOrderId;
    tr.price = tradePrice;
    tr.quantity = tradeQuantity;
    tr.time = "Just now";
    fallbackTrades.push_back(tr);

    if (!connected || !conn)
    {
        return true;
    }

    string query =
        "INSERT INTO Trades "
        "(buy_order_id, sell_order_id, trade_price, trade_quantity) VALUES (" +
        to_string(buyOrderId) + ", " +
        to_string(sellOrderId) + ", " +
        to_string(tradePrice) + ", " +
        to_string(tradeQuantity) + ")";

    if (mysql_query(conn, query.c_str()) == 0)
    {
        return true;
    }

    cout << "Trade could not be saved to DB: "
         << mysql_error(conn) << endl;

    return false;
}

bool Database::loadOrders(OrderBook& orderBook)
{
    if (!connected || !conn)
    {
        return false;
    }

    string query =
        "SELECT order_id, rate, qty, original_qty, "
        "order_no, side, status, trader "
        "FROM Orders "
        "WHERE status = 'ACTIVE' OR status = 'PARTIALLY FILLED'";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Could not load orders: "
             << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) return false;
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)))
    {
        int id = stoi(row[0]);
        double price = stod(row[1]);
        int quantity = stoi(row[2]);
        int orderNo = stoi(row[4]);
        string side = row[5];
        string trader = row[7];
        int originalQuantity = stoi(row[3]);
        string status = row[6];
        
        if (side == "BUY")
        {
          BuyOrder order(id, price, quantity, orderNo, trader);
          order.setoriginalqty(originalQuantity);

          if (status == "PARTIALLY FILLED")
            order.setstatus(Orderstatus::PARTIALLY_FILLED);
        
           orderBook.addbuy(order);
        }
        else if (side == "SELL")
        {
          SellOrder order(id, price, quantity, orderNo, trader);
          order.setoriginalqty(originalQuantity);

          if (status == "PARTIALLY FILLED")
           order.setstatus(Orderstatus::PARTIALLY_FILLED);

          orderBook.addsell(order);
        }
    }

    mysql_free_result(result);

    return true;
}

void Database::showTradeHistory()
{
    vector<TradeRecord> trades = getTradeHistory();

    cout << "\nTRADE HISTORY\n";
    if (trades.empty())
    {
        cout << "No executed trades yet.\n";
        return;
    }

    for (const auto& t : trades)
    {
        cout << "\nTrade ID: " << t.tradeId << endl;
        cout << "Buy Order ID: " << t.buyOrderId << endl;
        cout << "Sell Order ID: " << t.sellOrderId << endl;
        cout << "Trade Price: " << t.price << endl;
        cout << "Trade Quantity: " << t.quantity << endl;
        cout << "Trade Time: " << t.time << endl;
    }
}

vector<TradeRecord> Database::getTradeHistory()
{
    if (!connected || !conn)
    {
        vector<TradeRecord> rev = fallbackTrades;
        reverse(rev.begin(), rev.end());
        return rev;
    }

    vector<TradeRecord> list;
    string query = "SELECT trade_id, buy_order_id, sell_order_id, trade_price, trade_quantity, trade_time FROM Trades ORDER BY trade_id DESC";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        vector<TradeRecord> rev = fallbackTrades;
        reverse(rev.begin(), rev.end());
        return rev;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) return fallbackTrades;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        TradeRecord tr;
        tr.tradeId = row[0] ? stoi(row[0]) : 0;
        tr.buyOrderId = row[1] ? stoi(row[1]) : 0;
        tr.sellOrderId = row[2] ? stoi(row[2]) : 0;
        tr.price = row[3] ? stod(row[3]) : 0.0;
        tr.quantity = row[4] ? stoi(row[4]) : 0;
        tr.time = row[5] ? row[5] : "";
        list.push_back(tr);
    }

    mysql_free_result(result);
    return list;
}

void Database::close()
{
    if (conn != NULL)
    {
        mysql_close(conn);
        conn = NULL;
    }
    connected = false;
}
