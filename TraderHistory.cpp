#include "TraderHistory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

static string trim(const string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

TraderHistory::TraderHistory(const string& dir) : directory(dir)
{
    ensureDirectoryExists();
}

void TraderHistory::ensureDirectoryExists() const
{
    try
    {
        if (!fs::exists(directory))
        {
            fs::create_directories(directory);
        }
    }
    catch (...)
    {
    }
}

string TraderHistory::sanitizeFilename(const string& name) const
{
    string clean = name;
    for (char& c : clean)
    {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
        {
            c = '_';
        }
    }
    if (clean.empty()) clean = "Anonymous";
    return clean;
}

string TraderHistory::getFilePath(const string& trader) const
{
    return directory + "/" + sanitizeFilename(trader) + ".txt";
}

string TraderHistory::getCurrentTimestamp() const
{
    auto now = chrono::system_clock::now();
    time_t nowTime = chrono::system_clock::to_time_t(now);
    tm localTm;
#if defined(_WIN32)
    localtime_s(&localTm, &nowTime);
#else
    localtime_r(&nowTime, &localTm);
#endif
    ostringstream oss;
    oss << put_time(&localTm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void TraderHistory::logOrderPlacement(const Order& order)
{
    ensureDirectoryExists();
    string trader = order.getTrader();
    string path = getFilePath(trader);

    bool isNew = !fs::exists(path) || fs::file_size(path) == 0;

    ofstream out(path, ios::app);
    if (!out.is_open()) return;

    if (isNew)
    {
        out << "========================================================================================\n";
        out << " Trader Order & Trade History: " << trader << "\n";
        out << " Created: " << getCurrentTimestamp() << "\n";
        out << "========================================================================================\n";
        out << "TIMESTAMP           | ACTION    | ORDER_ID | SIDE | PRICE    | QTY | COUNTERPARTY | DETAILS\n";
        out << "----------------------------------------------------------------------------------------\n";
    }

    ostringstream priceStream;
    priceStream << fixed << setprecision(2) << order.getrate();

    ostringstream details;
    details << "Placed " << order.getsidestring() << " order #" << order.getid()
            << " @ " << priceStream.str() << " (Qty: " << order.getqty() << ")";

    out << getCurrentTimestamp() << " | "
        << "PLACED" << " | "
        << order.getid() << " | "
        << order.getsidestring() << " | "
        << priceStream.str() << " | "
        << order.getqty() << " | "
        << "-" << " | "
        << details.str() << "\n";

    out.close();
}

void TraderHistory::logTradeExecution(const string& trader, const string& side, int orderId,
                                     double price, int quantity, const string& counterparty, int counterpartyOrderId)
{
    ensureDirectoryExists();
    string path = getFilePath(trader);

    bool isNew = !fs::exists(path) || fs::file_size(path) == 0;

    ofstream out(path, ios::app);
    if (!out.is_open()) return;

    if (isNew)
    {
        out << "========================================================================================\n";
        out << " Trader Order & Trade History: " << trader << "\n";
        out << " Created: " << getCurrentTimestamp() << "\n";
        out << "========================================================================================\n";
        out << "TIMESTAMP           | ACTION    | ORDER_ID | SIDE | PRICE    | QTY | COUNTERPARTY | DETAILS\n";
        out << "----------------------------------------------------------------------------------------\n";
    }

    ostringstream priceStream;
    priceStream << fixed << setprecision(2) << price;

    ostringstream counterpartyStr;
    counterpartyStr << counterparty << " (#" << counterpartyOrderId << ")";

    ostringstream details;
    details << "Trade executed with " << counterparty << " (#" << counterpartyOrderId << ")"
            << " @ " << priceStream.str() << " (Matched Qty: " << quantity << ")";

    out << getCurrentTimestamp() << " | "
        << "EXECUTED" << " | "
        << orderId << " | "
        << side << " | "
        << priceStream.str() << " | "
        << quantity << " | "
        << counterpartyStr.str() << " | "
        << details.str() << "\n";

    out.close();
}

void TraderHistory::logOrderCancellation(const string& trader, int orderId, const string& side, double price, int qty)
{
    ensureDirectoryExists();
    string path = getFilePath(trader);
    if (!fs::exists(path)) return;

    ofstream out(path, ios::app);
    if (!out.is_open()) return;

    ostringstream priceStream;
    priceStream << fixed << setprecision(2) << price;

    ostringstream details;
    details << "Order #" << orderId << " (" << side << " @ " << priceStream.str() << ") cancelled";

    out << getCurrentTimestamp() << " | "
        << "CANCELLED" << " | "
        << orderId << " | "
        << side << " | "
        << priceStream.str() << " | "
        << qty << " | "
        << "-" << " | "
        << details.str() << "\n";

    out.close();
}

void TraderHistory::logOrderModification(const string& trader, int orderId, double newPrice, int newQuantity)
{
    ensureDirectoryExists();
    string path = getFilePath(trader);
    if (!fs::exists(path)) return;

    ofstream out(path, ios::app);
    if (!out.is_open()) return;

    ostringstream priceStream;
    priceStream << fixed << setprecision(2) << newPrice;

    ostringstream details;
    details << "Order #" << orderId << " modified to Price: " << priceStream.str() << ", Qty: " << newQuantity;

    out << getCurrentTimestamp() << " | "
        << "MODIFIED" << " | "
        << orderId << " | "
        << "-" << " | "
        << priceStream.str() << " | "
        << newQuantity << " | "
        << "-" << " | "
        << details.str() << "\n";

    out.close();
}

bool TraderHistory::hasHistory(const string& trader) const
{
    string path = getFilePath(trader);
    return fs::exists(path) && fs::file_size(path) > 0;
}

vector<string> TraderHistory::getRawLines(const string& trader) const
{
    vector<string> lines;
    string path = getFilePath(trader);

    ifstream in(path);
    if (!in.is_open()) return lines;

    string line;
    while (getline(in, line))
    {
        lines.push_back(line);
    }
    in.close();

    return lines;
}

void TraderHistory::printHistory(const string& trader) const
{
    string path = getFilePath(trader);
    ifstream in(path);

    if (!in.is_open())
    {
        cout << "\n[File Handling] No history file found for trader: " << trader << "\n";
        cout << "                File checked: " << path << "\n";
        return;
    }

    cout << "\n--- READING FROM FILE: " << path << " ---\n";
    string line;
    while (getline(in, line))
    {
        cout << line << "\n";
    }
    cout << "--- END OF FILE ---\n\n";

    in.close();
}

vector<TraderHistoryEntry> TraderHistory::getHistory(const string& trader) const
{
    vector<TraderHistoryEntry> entries;
    string path = getFilePath(trader);

    ifstream in(path);
    if (!in.is_open()) return entries;

    string line;
    while (getline(in, line))
    {
        string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '=' || trimmed[0] == '-' || trimmed.find("TIMESTAMP") != string::npos)
        {
            continue;
        }

        stringstream ss(line);
        string token;
        vector<string> parts;
        while (getline(ss, token, '|'))
        {
            parts.push_back(trim(token));
        }

        if (parts.size() >= 8)
        {
            TraderHistoryEntry entry;
            entry.timestamp = parts[0];
            entry.action = parts[1];
            try { entry.orderId = stoi(parts[2]); } catch (...) { entry.orderId = 0; }
            entry.side = parts[3];
            try { entry.price = stod(parts[4]); } catch (...) { entry.price = 0.0; }
            try { entry.quantity = stoi(parts[5]); } catch (...) { entry.quantity = 0; }
            entry.counterparty = parts[6];
            entry.details = parts[7];
            entries.push_back(entry);
        }
    }

    in.close();
    return entries;
}

vector<string> TraderHistory::getAllTraders() const
{
    vector<string> traders;
    ensureDirectoryExists();

    try
    {
        for (const auto& entry : fs::directory_iterator(directory))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".txt")
            {
                traders.push_back(entry.path().stem().string());
            }
        }
    }
    catch (...)
    {
    }

    sort(traders.begin(), traders.end());
    return traders;
}

