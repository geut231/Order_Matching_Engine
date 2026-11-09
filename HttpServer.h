#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include "MatchingEngine.h"
#include "Database.h"

class HttpServer
{
private:
    int port;
    std::atomic<bool> running;
    SOCKET listenSocket;
    std::thread serverThread;
    std::mutex engineMutex;
    MatchingEngine* engine;
    Database* db;

    void run();
    void handleClient(SOCKET clientSocket);

    std::string handleRequest(const std::string& method, const std::string& path, const std::string& body);
    std::string getContentType(const std::string& path);
    std::string readFile(const std::string& filepath);

    // API Handlers
    std::string handleGetMarketSummary();
    std::string handleGetOrderBook();
    std::string handleGetActiveOrders();
    std::string handleGetTrades();
    std::string handlePostOrder(const std::string& body);
    std::string handlePostCancel(const std::string& body);
    std::string handlePostModify(const std::string& body);

public:
    HttpServer(MatchingEngine* eng, Database* database, int p = 8080);
    ~HttpServer();

    bool start();
    void stop();
    int getPort() const { return port; }
    bool isRunning() const { return running.load(); }
};

#endif

