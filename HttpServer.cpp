#include "HttpServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// JSON Helpers
static string escapeJson(const string& str)
{
    ostringstream oss;
    for (char c : str)
    {
        switch (c)
        {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:   oss << c; break;
        }
    }
    return oss.str();
}

static string extractJsonString(const string& json, const string& key)
{
    string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == string::npos) return "";
    pos = json.find(':', pos);
    if (pos == string::npos) return "";
    pos = json.find('"', pos);
    if (pos == string::npos) return "";
    size_t end = json.find('"', pos + 1);
    if (end == string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}

static double extractJsonDouble(const string& json, const string& key)
{
    string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == string::npos) return 0.0;
    pos = json.find(':', pos);
    if (pos == string::npos) return 0.0;
    size_t start = json.find_first_of("0123456789.-", pos);
    if (start == string::npos) return 0.0;
    size_t end = json.find_first_not_of("0123456789.eE+-", start);
    string val = (end == string::npos) ? json.substr(start) : json.substr(start, end - start);
    try { return stod(val); } catch (...) { return 0.0; }
}

static int extractJsonInt(const string& json, const string& key)
{
    return static_cast<int>(extractJsonDouble(json, key));
}

HttpServer::HttpServer(MatchingEngine* eng, Database* database, int p)
    : port(p), running(false), listenSocket(INVALID_SOCKET), engine(eng), db(database)
{
}

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::start()
{
    if (running.load()) return true;

    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
        cerr << "[API] WSAStartup failed: " << res << endl;
        return false;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        cerr << "[API] socket creation failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return false;
    }

    // Set SO_REUSEADDR
    BOOL opt = TRUE;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    if (::bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "[API] bind failed on port " << port << ": " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "[API] listen failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    running.store(true);
    serverThread = std::thread(&HttpServer::run, this);

    return true;
}

void HttpServer::stop()
{
    if (!running.load()) return;

    running.store(false);

    if (listenSocket != INVALID_SOCKET)
    {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }

    if (serverThread.joinable())
    {
        serverThread.join();
    }

    WSACleanup();
}

void HttpServer::run()
{
    while (running.load())
    {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == INVALID_SOCKET)
        {
            if (!running.load()) break;
            continue;
        }

        std::thread(&HttpServer::handleClient, this, clientSocket).detach();
    }
}

void HttpServer::handleClient(SOCKET clientSocket)
{
    vector<char> buffer(8192);
    int bytesRead = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()) - 1, 0);

    if (bytesRead <= 0)
    {
        closesocket(clientSocket);
        return;
    }

    buffer[bytesRead] = '\0';
    string requestRaw(buffer.data(), bytesRead);

    // Parse HTTP Request Line: METHOD PATH HTTP/1.x
    istringstream reqStream(requestRaw);
    string method, path, version;
    reqStream >> method >> path >> version;

    // Remove query params if any
    size_t queryPos = path.find('?');
    if (queryPos != string::npos)
    {
        path = path.substr(0, queryPos);
    }

    // Extract body
    string body = "";
    size_t headerEnd = requestRaw.find("\r\n\r\n");
    if (headerEnd != string::npos)
    {
        body = requestRaw.substr(headerEnd + 4);
    }

    string response = handleRequest(method, path, body);
    send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
    closesocket(clientSocket);
}

string HttpServer::getContentType(const string& path)
{
    if (path.rfind(".html") != string::npos || path == "/") return "text/html; charset=utf-8";
    if (path.rfind(".css") != string::npos) return "text/css; charset=utf-8";
    if (path.rfind(".js") != string::npos) return "application/javascript; charset=utf-8";
    if (path.rfind(".json") != string::npos) return "application/json; charset=utf-8";
    return "text/plain; charset=utf-8";
}

string HttpServer::readFile(const string& filepath)
{
    ifstream file(filepath, ios::binary);
    if (!file.is_open()) return "";
    ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

string HttpServer::handleRequest(const string& method, const string& path, const string& body)
{
    // Handle CORS preflight
    if (method == "OPTIONS")
    {
        return "HTTP/1.1 204 No Content\r\n"
               "Access-Control-Allow-Origin: *\r\n"
               "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n"
               "Access-Control-Allow-Headers: Content-Type\r\n"
               "Connection: close\r\n\r\n";
    }

    // Static files
    if (method == "GET")
    {
        if (path == "/" || path == "/index.html")
        {
            string content = readFile("index.html");
            if (!content.empty())
            {
                return "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html; charset=utf-8\r\n"
                       "Content-Length: " + to_string(content.size()) + "\r\n"
                       "Access-Control-Allow-Origin: *\r\n"
                       "Connection: close\r\n\r\n" + content;
            }
        }
        else if (path == "/style.css")
        {
            string content = readFile("style.css");
            if (!content.empty())
            {
                return "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/css; charset=utf-8\r\n"
                       "Content-Length: " + to_string(content.size()) + "\r\n"
                       "Access-Control-Allow-Origin: *\r\n"
                       "Connection: close\r\n\r\n" + content;
            }
        }
        else if (path == "/app.js")
        {
            string content = readFile("app.js");
            if (!content.empty())
            {
                return "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/javascript; charset=utf-8\r\n"
                       "Content-Length: " + to_string(content.size()) + "\r\n"
                       "Access-Control-Allow-Origin: *\r\n"
                       "Connection: close\r\n\r\n" + content;
            }
        }
        // API Routes
        else if (path == "/api/market-summary")
        {
            return handleGetMarketSummary();
        }
        else if (path == "/api/order-book")
        {
            return handleGetOrderBook();
        }
        else if (path == "/api/active-orders")
        {
            return handleGetActiveOrders();
        }
        else if (path == "/api/trades")
        {
            return handleGetTrades();
        }
    }
    else if (method == "POST")
    {
        if (path == "/api/orders")
        {
            return handlePostOrder(body);
        }
        else if (path == "/api/cancel")
        {
            return handlePostCancel(body);
        }
        else if (path == "/api/modify")
        {
            return handlePostModify(body);
        }
    }

    // 404 Not Found
    string notFound = "{\"error\": \"Not Found\", \"path\": \"" + escapeJson(path) + "\"}";
    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(notFound.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + notFound;
}

string HttpServer::handleGetMarketSummary()
{
    lock_guard<mutex> lock(engineMutex);

    double bestBid = engine->getOrderBook().getBestBid();
    double bestAsk = engine->getOrderBook().getBestAsk();
    double spread = engine->getOrderBook().getSpread();
    double midPrice = engine->getOrderBook().getMidPrice();

    vector<Order> buys = engine->getOrderBook().getBuys();
    vector<Order> sells = engine->getOrderBook().getSells();
    int activeOrdersCount = static_cast<int>(buys.size() + sells.size());

    vector<TradeRecord> trades = db->getTradeHistory();
    int tradeCount = static_cast<int>(trades.size());
    int totalQty = 0;
    double totalValue = 0.0;
    for (const auto& t : trades)
    {
        totalQty += t.quantity;
        totalValue += (t.price * t.quantity);
    }

    ostringstream json;
    json << fixed << setprecision(2);
    json << "{"
         << "\"best_bid\":" << bestBid << ","
         << "\"best_ask\":" << bestAsk << ","
         << "\"spread\":" << spread << ","
         << "\"mid_price\":" << midPrice << ","
         << "\"active_orders\":" << activeOrdersCount << ","
         << "\"executed_trades\":" << tradeCount << ","
         << "\"traded_quantity\":" << totalQty << ","
         << "\"traded_value\":" << totalValue
         << "}";

    string res = json.str();
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(res.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + res;
}

string HttpServer::handleGetOrderBook()
{
    lock_guard<mutex> lock(engineMutex);

    vector<Order> buys = engine->getOrderBook().getBuys();
    vector<Order> sells = engine->getOrderBook().getSells();

    ostringstream json;
    json << fixed << setprecision(2);
    json << "{\"buys\":[";
    for (size_t i = 0; i < buys.size(); ++i)
    {
        if (i > 0) json << ",";
        json << "{"
             << "\"id\":" << buys[i].getid() << ","
             << "\"price\":" << buys[i].getrate() << ","
             << "\"quantity\":" << buys[i].getqty() << ","
             << "\"order_no\":" << buys[i].getno() << ","
             << "\"trader\":\"" << escapeJson(buys[i].getTrader()) << "\""
             << "}";
    }
    json << "],\"sells\":[";
    for (size_t i = 0; i < sells.size(); ++i)
    {
        if (i > 0) json << ",";
        json << "{"
             << "\"id\":" << sells[i].getid() << ","
             << "\"price\":" << sells[i].getrate() << ","
             << "\"quantity\":" << sells[i].getqty() << ","
             << "\"order_no\":" << sells[i].getno() << ","
             << "\"trader\":\"" << escapeJson(sells[i].getTrader()) << "\""
             << "}";
    }
    json << "]}";

    string res = json.str();
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(res.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + res;
}

string HttpServer::handleGetActiveOrders()
{
    lock_guard<mutex> lock(engineMutex);

    vector<Order> buys = engine->getOrderBook().getBuys();
    vector<Order> sells = engine->getOrderBook().getSells();

    vector<Order> allOrders;
    allOrders.insert(allOrders.end(), buys.begin(), buys.end());
    allOrders.insert(allOrders.end(), sells.begin(), sells.end());

    ostringstream json;
    json << fixed << setprecision(2);
    json << "[";
    for (size_t i = 0; i < allOrders.size(); ++i)
    {
        if (i > 0) json << ",";
        json << "{"
             << "\"id\":" << allOrders[i].getid() << ","
             << "\"trader\":\"" << escapeJson(allOrders[i].getTrader()) << "\","
             << "\"side\":\"" << allOrders[i].getsidestring() << "\","
             << "\"price\":" << allOrders[i].getrate() << ","
             << "\"quantity\":" << allOrders[i].getqty() << ","
             << "\"original_qty\":" << allOrders[i].getoriginalqty() << ","
             << "\"status\":\"" << allOrders[i].getstatusstring() << "\""
             << "}";
    }
    json << "]";

    string res = json.str();
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(res.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + res;
}

string HttpServer::handleGetTrades()
{
    lock_guard<mutex> lock(engineMutex);

    vector<TradeRecord> trades = db->getTradeHistory();

    ostringstream json;
    json << fixed << setprecision(2);
    json << "[";
    for (size_t i = 0; i < trades.size(); ++i)
    {
        if (i > 0) json << ",";
        double val = trades[i].price * trades[i].quantity;
        json << "{"
             << "\"trade_id\":" << trades[i].tradeId << ","
             << "\"buy_order_id\":" << trades[i].buyOrderId << ","
             << "\"sell_order_id\":" << trades[i].sellOrderId << ","
             << "\"price\":" << trades[i].price << ","
             << "\"quantity\":" << trades[i].quantity << ","
             << "\"value\":" << val << ","
             << "\"time\":\"" << escapeJson(trades[i].time) << "\""
             << "}";
    }
    json << "]";

    string res = json.str();
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(res.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + res;
}

string HttpServer::handlePostOrder(const string& body)
{
    string side = extractJsonString(body, "side");
    string trader = extractJsonString(body, "trader");
    double price = extractJsonDouble(body, "price");
    int quantity = extractJsonInt(body, "quantity");

    if (trader.empty()) trader = "Anonymous";

    // Normalize side
    transform(side.begin(), side.end(), side.begin(), ::toupper);

    if (price <= 0 || quantity <= 0 || (side != "BUY" && side != "SELL"))
    {
        string err = "{\"success\": false, \"message\": \"Invalid parameters: price > 0, quantity > 0, side must be BUY or SELL.\"}";
        return "HTTP/1.1 400 Bad Request\r\n"
               "Content-Type: application/json; charset=utf-8\r\n"
               "Content-Length: " + to_string(err.size()) + "\r\n"
               "Access-Control-Allow-Origin: *\r\n"
               "Connection: close\r\n\r\n" + err;
    }

    bool success = false;
    {
        lock_guard<mutex> lock(engineMutex);
        if (side == "BUY")
        {
            success = engine->placeBuyOrder(price, quantity, trader);
        }
        else
        {
            success = engine->placeSellOrder(price, quantity, trader);
        }
    }

    ostringstream json;
    json << "{"
         << "\"success\":" << (success ? "true" : "false") << ","
         << "\"message\":\"" << (success ? "Order placed and matching executed." : "Failed to place order in engine/database.") << "\""
         << "}";

    string res = json.str();
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(res.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + res;
}

string HttpServer::handlePostCancel(const string& body)
{
    int id = extractJsonInt(body, "id");
    if (id <= 0)
    {
        string err = "{\"success\": false, \"message\": \"Invalid order ID.\"}";
        return "HTTP/1.1 400 Bad Request\r\n"
               "Content-Type: application/json; charset=utf-8\r\n"
               "Content-Length: " + to_string(err.size()) + "\r\n"
               "Access-Control-Allow-Origin: *\r\n"
               "Connection: close\r\n\r\n" + err;
    }

    bool success = false;
    {
        lock_guard<mutex> lock(engineMutex);
        success = engine->cancelOrder(id);
    }

    ostringstream json;
    json << "{"
         << "\"success\":" << (success ? "true" : "false") << ","
         << "\"message\":\"" << (success ? "Order cancelled successfully." : "Active order not found.") << "\""
         << "}";

    string res = json.str();
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(res.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + res;
}

string HttpServer::handlePostModify(const string& body)
{
    int id = extractJsonInt(body, "id");
    double price = extractJsonDouble(body, "price");
    int quantity = extractJsonInt(body, "quantity");

    if (id <= 0 || price <= 0 || quantity <= 0)
    {
        string err = "{\"success\": false, \"message\": \"Invalid parameters for modification.\"}";
        return "HTTP/1.1 400 Bad Request\r\n"
               "Content-Type: application/json; charset=utf-8\r\n"
               "Content-Length: " + to_string(err.size()) + "\r\n"
               "Access-Control-Allow-Origin: *\r\n"
               "Connection: close\r\n\r\n" + err;
    }

    bool success = false;
    {
        lock_guard<mutex> lock(engineMutex);
        success = engine->modifyOrder(id, price, quantity);
    }

    ostringstream json;
    json << "{"
         << "\"success\":" << (success ? "true" : "false") << ","
         << "\"message\":\"" << (success ? "Order modified successfully." : "Order could not be modified.") << "\""
         << "}";

    string res = json.str();
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Content-Length: " + to_string(res.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n\r\n" + res;
}

