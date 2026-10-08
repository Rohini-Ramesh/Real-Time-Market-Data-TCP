#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <nlohmann/json.hpp>

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 8080

using json = nlohmann::json;

struct PriceTick
{
    std::string symbol;
    double price;
    std::string timestamp;
};

struct PriceStatistics
{
    int count = 0;

    double minPrice = 0.0;
    double maxPrice = 0.0;

    double totalPrice = 0.0;

    double averagePrice = 0.0;
};

// Parse one complete price tick
bool parseTick(const std::string& line, PriceTick& tick)
{
    std::stringstream stream(line);

    std::string symbol;
    std::string priceString;
    std::string timestamp;


    // Extract symbol
    if (!std::getline(stream, symbol, ','))
    {
        return false;
    }


    // Extract price
    if (!std::getline(stream, priceString, ','))
    {
        return false;
    }


    // Extract timestamp
    if (!std::getline(stream, timestamp))
    {
        return false;
    }


    // Basic validation
    if (symbol.empty() ||
        priceString.empty() ||
        timestamp.empty())
    {
        return false;
    }


    // Convert price string to double
    try
    {
        tick.price = std::stod(priceString);
    }
    catch (...)
    {
        return false;
    }


    tick.symbol = symbol;
    tick.timestamp = timestamp;


    // Validate price
    if (tick.price <= 0)
    {
        return false;
    }


    return true;
}


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr
            << "Usage: " << argv[0]
            << " <server-ip>\n";

        return 1;
    }

    std::string serverIP = argv[1];
    // ---------------------------------------
    // 1. Create socket
    // ---------------------------------------

    int clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );


    if (clientSocket < 0)
    {
        std::cout << "Socket creation failed\n";
        return 1;
    }


    std::cout << "Socket created successfully\n";


    // ---------------------------------------
    // 2. Configure server address
    // ---------------------------------------

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);


    if (inet_pton(
        AF_INET,
        serverIP.c_str(),
        &serverAddress.sin_addr
    ) != 1)
{
    std::cerr
        << "Invalid IPv4 address: "
        << serverIP << "\n";

    close(clientSocket);
    return 1;
}


    // ---------------------------------------
    // 3. Connect
    // ---------------------------------------

    if (connect(
        clientSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) < 0)
    {
        perror("Connection failed");

        close(clientSocket);

        return 1;
    }


    std::cout
        << "Connected to server successfully!\n\n";
     // Connect to the local Python dashboard bridge
int dashboardSocket = socket(AF_INET, SOCK_STREAM, 0);

if (dashboardSocket < 0)
{
    std::cerr << "Failed to create dashboard socket\n";
}
else
{
    sockaddr_in dashboardAddress{};
    dashboardAddress.sin_family = AF_INET;
    dashboardAddress.sin_port = htons(9090);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &dashboardAddress.sin_addr
    );

    if (connect(
        dashboardSocket,
        (sockaddr*)&dashboardAddress,
        sizeof(dashboardAddress)
    ) < 0)
    {
        perror("Dashboard bridge connection failed");
        close(dashboardSocket);
        dashboardSocket = -1;
    }
    else
    {
        std::cout << "Connected to dashboard bridge!\n";
    }
}


    // ---------------------------------------
    // 4. Receive TCP stream
    // ---------------------------------------

    char buffer[1024];

    std::string pendingData;

    std::map<std::string, double> previousPrices;

    std::map<std::string, PriceStatistics> statistics;

    int totalTicks = 0;
    int invalidTicks = 0;
    int bytesReceived;


    while ((bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer),
        0
    )) > 0)
    {
        // Add received bytes to our buffer
        pendingData.append(
            buffer,
            bytesReceived
        );


        // Look for complete messages
        size_t newlinePosition;


        while (
            (newlinePosition =
                pendingData.find('\n'))
            != std::string::npos
        )
        {
            // Extract one complete line
            std::string line =
                pendingData.substr(
                    0,
                    newlinePosition
                );


            // Remove processed line
            pendingData.erase(
                0,
                newlinePosition + 1
            );


            // ---------------------------------------
            // 5. Parse price tick
            // ---------------------------------------

            PriceTick tick;


            if (parseTick(line, tick))
            {
                //check whether we have seen this stock before
                bool hasPreviousPrice =
                    previousPrices.find(tick.symbol)
                    != previousPrices.end();


                double previousPrice = tick.price;

                double change = 0.0;
                double changePercent = 0.0;


                if (hasPreviousPrice)
                {   
                    previousPrice =
                        previousPrices[tick.symbol];

                    change =
                        tick.price - previousPrice;

                    changePercent =
                        (change / previousPrice) * 100.0;
                }


                previousPrices[tick.symbol] =
                    tick.price;

                totalTicks++;

                PriceStatistics& stats =
                    statistics[tick.symbol];

                stats.count++;

                stats.totalPrice += tick.price;

                if (stats.count == 1)
                {
                    stats.minPrice = tick.price;
                    stats.maxPrice = tick.price;
                }
                else
                {
                    if (tick.price < stats.minPrice)
                    {
                        stats.minPrice = tick.price;
                    }

                    if (tick.price > stats.maxPrice)
                    {
                        stats.maxPrice = tick.price;
                    }
                }

                stats.averagePrice =
                    stats.totalPrice / stats.count;

                  // Prepare structured data for the dashboard
                  json tickData = {
   		     {"symbol", tick.symbol},
   		     {"price", tick.price},
   		     {"previousPrice", hasPreviousPrice ? previousPrice : 0.0},
   		     {"hasPreviousPrice", hasPreviousPrice},
   		     {"change", change},
   		     {"changePercent", changePercent},
   		     {"timestamp", tick.timestamp},
   		     {"count", stats.count},
   		     {"minPrice", stats.minPrice},
   		     {"maxPrice", stats.maxPrice},
   		     {"averagePrice", stats.averagePrice},
   		     {"totalTicks", totalTicks},
   		     {"invalidTicks", invalidTicks}
		     };
// Forward the processed tick to the dashboard
if (dashboardSocket >= 0)
{
    std::string jsonMessage = tickData.dump() + "\n";

    size_t totalSent = 0;

    while (totalSent < jsonMessage.size())
    {
        ssize_t sent = send(
            dashboardSocket,
            jsonMessage.data() + totalSent,
            jsonMessage.size() - totalSent,
            MSG_NOSIGNAL
        );

        if (sent <= 0)
        {
            std::cerr << "Dashboard connection lost\n";
            close(dashboardSocket);
            dashboardSocket = -1;
            break;
        }

        totalSent += static_cast<size_t>(sent);
    }
}

                // ---------------------------------------
                // Display statistics every 10 ticks
                // ---------------------------------------    
                
                if (totalTicks % 10 == 0)
                {
                    std::cout
                        << "\n\n";
        
                    std::cout
                        << "========================================\n";

                    std::cout
                        << "          STREAM STATISTICS\n";

                    std::cout
                        << "========================================\n";

                    std::cout
                        << "Total ticks   : "
                        << totalTicks
                        << "\n";

                    std::cout
                        << "Invalid ticks : "
                        << invalidTicks
                        << "\n\n";


                    for (const auto& entry : statistics)
                    {
                        const std::string& symbol =
                            entry.first;

                        const PriceStatistics& s =
                            entry.second;

                        std::cout
                            << symbol
                            << "\n";

                        std::cout
                            << "  Count   : "
                            << s.count
                            << "\n";

                        std::cout
                            << "  Min     : $"
                            << std::fixed
                            << std::setprecision(2)
                            << s.minPrice
                            << "\n";

                        std::cout
                            << "  Max     : $"
                            << s.maxPrice
                            << "\n";

                        std::cout
                            << "  Average : $"
                            << s.averagePrice
                            << "\n\n";
                    }

                    std::cout
                        << "========================================\n\n";
                }
                //----------------------------
                //Display parsed price tick
                //----------------------------

                std::cout
                    << "========================================\n";

                std::cout
                    << "          MARKET PRICE TICK\n";

                std::cout
                    << "========================================\n";

                std::cout
                    << "Ticker    : "
                    << tick.symbol
                    << "\n";

                std::cout
                    << "Price     : $"
                    << std::fixed
                    << std::setprecision(2)
                    << tick.price
                    << "\n";

                if (hasPreviousPrice)
                {
                    std::cout
                        << "Previous  : $"
                        << previousPrice
                        << "\n";

                    std::cout
                        << "Change    : "
                        << (change >= 0 ? "+" : "")
                        << change
                        << "\n";

                    std::cout
                        << "Change %  : "
                        << (changePercent >= 0 ? "+" : "")
                        << changePercent
                        << "%\n";
                }
                else
                {
                    std::cout
                        << "Previous  : N/A\n";

                    std::cout
                        << "Change    : N/A\n";

                    std::cout
                        << "Change %  : N/A\n";
                }

                std::cout
                    << "Timestamp : "
                    << tick.timestamp
                    << "\n";

                std::cout
                    << "========================================\n\n";
            }
            else
            {
                invalidTicks++;

                std::cout
                    << "INVALID PRICE TICK: "
                    << line
                    << "\n";
            }
        }
    }


    // ---------------------------------------
    // 6. Connection closed
    // ---------------------------------------

    std::cout
        << "\nServer disconnected.\n";

if (dashboardSocket >= 0)
{
    close(dashboardSocket);
}


    close(clientSocket);


    return 0;
}
