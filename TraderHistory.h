#ifndef TRADERHISTORY_H
#define TRADERHISTORY_H

#include <string>
#include <vector>
#include "Order.h"

struct TraderHistoryEntry
{
    std::string timestamp;
    std::string action;       // PLACED, EXECUTED, CANCELLED, MODIFIED
    int orderId;
    std::string side;         // BUY, SELL, or "-"
    double price;
    int quantity;
    std::string counterparty; // Counterparty trader name (for trades)
    std::string details;
};

class TraderHistory
{
private:
    std::string directory;

    std::string sanitizeFilename(const std::string& name) const;
    std::string getFilePath(const std::string& trader) const;
    std::string getCurrentTimestamp() const;
    void ensureDirectoryExists() const;

public:
    TraderHistory(const std::string& dir = "user_history");

    // File Writing Operations (ofstream)
    void logOrderPlacement(const Order& order);
    void logTradeExecution(const std::string& trader, const std::string& side, int orderId,
                           double price, int quantity, const std::string& counterparty, int counterpartyOrderId);
    void logOrderCancellation(const std::string& trader, int orderId, const std::string& side, double price, int qty);
    void logOrderModification(const std::string& trader, int orderId, double newPrice, int newQuantity);

    // File Reading Operations (ifstream)
    bool hasHistory(const std::string& trader) const;
    std::vector<TraderHistoryEntry> getHistory(const std::string& trader) const;
    std::vector<std::string> getRawLines(const std::string& trader) const;
    void printHistory(const std::string& trader) const;
    std::vector<std::string> getAllTraders() const;
};

#endif

