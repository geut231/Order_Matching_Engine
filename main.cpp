#include <iostream>
#include <string>
#include <limits>
#include "MatchingEngine.h"
#include "Database.h"
#include "HttpServer.h"
using namespace std;

int main()
{   
    Database db;

    if (!db.connect())
    {
        return 1;
    }

    MatchingEngine engine(&db);

    // Launch Native C++ HTTP API Server
    HttpServer server(&engine, &db, 8080);
    if (server.start())
    {
        cout << "\n=======================================================\n";
        cout << "  HTTP REST API & Web Dashboard Started!\n";
        cout << "  Open: http://localhost:8080\n";
        cout << "=======================================================\n";
    }
    int choice = -1;

    do
    {
        cout << "\nORDER MATCHING ENGINE\n";
        cout << "1. Place Buy Order\n";
        cout << "2. Place Sell Order\n";
        cout << "3. Show Order Book\n";
        cout << "4. Cancel Order\n";
        cout << "5. Modify Order\n";
        cout << "6. Search Active Order\n";
        cout << "7. Process Matching\n";
        cout << "8. Show Trade History\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice))
        {
            cout << "Invalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1 || choice == 2)
        {
            double price;
            int quantity;
            string trader;

            cout << "Enter price: ";
            if (!(cin >> price))
            {
                cout << "Invalid price.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            cout << "Enter quantity: ";
            if (!(cin >> quantity))
            {
                cout << "Invalid quantity.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            cout << "Enter trader name: ";
            cin >> trader;

            bool placed = (choice == 1)
                              ? engine.placeBuyOrder(price, quantity, trader)
                              : engine.placeSellOrder(price, quantity, trader);

            if (placed)
                cout << "Order accepted.\n";
            else
                cout << "Order rejected. Check price, quantity and trader name.\n";
        }
        else if (choice == 3)
        {
            engine.showOrderBook();
        }
        else if (choice == 4)
        {
            int id;
            cout << "Enter order ID to cancel: ";
            cin >> id;
            cout << (engine.cancelOrder(id) ? "Order cancelled.\n" : "Active order not found.\n");
        }
        else if (choice == 5)
        {
            int id;
            double price;
            int quantity;
            cout << "Enter order ID to modify: ";
            cin >> id;
            cout << "Enter new price: ";
            cin >> price;
            cout << "Enter new quantity: ";
            cin >> quantity;
            cout << (engine.modifyOrder(id, price, quantity)
                         ? "Order modified.\n"
                         : "Order could not be modified.\n");
        }
        else if (choice == 6)
        {
            int id;
            cout << "Enter order ID to search: ";
            cin >> id;
            Order result(1, 1.0, 1, 1, Orderside::BUY, "placeholder");
            if (engine.searchOrder(id, result))
                result.show();
            else
                cout << "Active order not found.\n";
        }
        else if (choice == 7)
        {
            engine.processMatching();
            cout << "Matching process completed.\n";
        }
        else if (choice == 8)
        {
            db.showTradeHistory();
        }
        else if (choice != 0)
        {
            cout << "Invalid menu choice.\n";
        }
    } while (choice != 0);

    server.stop();
    cout << "Exiting Order Matching Engine.\n";
    return 0;
}
