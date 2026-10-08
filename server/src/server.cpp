#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#define PORT 8080

// Send the complete message, including handling partial sends.
bool sendAll(SOCKET clientSocket, const std::string& message)
{
    int totalSent = 0;
    int messageSize = static_cast<int>(message.size());

    while (totalSent < messageSize)
    {
        int bytesSent = send(
            clientSocket,
            message.c_str() + totalSent,
            messageSize - totalSent,
            0
        );

        if (bytesSent == SOCKET_ERROR)
        {
            std::cout
                << "Send failed. Socket error: "
                << WSAGetLastError() << "\n";

            return false;
        }

        if (bytesSent == 0)
        {
            std::cout << "Client connection closed.\n";
            return false;
        }

        totalSent += bytesSent;
    }

    return true;
}

int main()
{
#ifdef _WIN32

    // ---------------------------------------
    // 1. Initialize Winsock
    // ---------------------------------------

    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    // ---------------------------------------
    // 2. Create server socket
    // ---------------------------------------

    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (serverSocket == INVALID_SOCKET)
    {
        std::cout
            << "Socket creation failed: "
            << WSAGetLastError() << "\n";

        WSACleanup();
        return 1;
    }

    std::cout << "Socket created successfully\n";

    // ---------------------------------------
    // 3. Configure server address
    // ---------------------------------------

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // ---------------------------------------
    // 4. Bind socket
    // ---------------------------------------

    if (bind(
        serverSocket,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    ) == SOCKET_ERROR)
    {
        std::cout
            << "Bind failed: "
            << WSAGetLastError() << "\n";

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server bound to port " << PORT << "\n";

    // ---------------------------------------
    // 5. Listen for clients
    // ---------------------------------------

    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        std::cout
            << "Listen failed: "
            << WSAGetLastError() << "\n";

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "\n";

    // ---------------------------------------
    // 6. Initialize simulated market data
    // ---------------------------------------

    std::vector<std::string> symbols = {
        "AAPL",
        "NVDA",
        "MSFT",
        "GOOG"
    };

    std::map<std::string, double> prices = {
        {"AAPL", 215.00},
        {"NVDA", 187.00},
        {"MSFT", 512.00},
        {"GOOG", 201.00}
    };

    std::random_device rd;
    std::mt19937 generator(rd());

    std::uniform_real_distribution<double> changeDistribution(
        -1.00,
        1.00
    );

    int symbolIndex = 0;

    // ---------------------------------------
    // 7. Accept and handle client connections
    // ---------------------------------------

    while (true)
    {
        std::cout
            << "\nWaiting for client connection...\n";

        SOCKET clientSocket = accept(
            serverSocket,
            nullptr,
            nullptr
        );

        if (clientSocket == INVALID_SOCKET)
        {
            std::cout
                << "Accept failed: "
                << WSAGetLastError() << "\n";

            break;
        }

        std::cout << "Client connected successfully!\n";

        // ---------------------------------------
        // 8. Stream simulated price ticks
        // ---------------------------------------

        while (true)
        {
            std::string symbol =
                symbols[symbolIndex % symbols.size()];

            symbolIndex++;

            double priceChange =
                changeDistribution(generator);

            prices[symbol] += priceChange;

            if (prices[symbol] < 1.0)
            {
                prices[symbol] = 1.0;
            }

            double price = prices[symbol];

            auto now = std::chrono::system_clock::now();

            std::time_t currentTime =
                std::chrono::system_clock::to_time_t(now);

            std::tm timeInfo{};

            localtime_s(&timeInfo, &currentTime);

            std::ostringstream timestampStream;

            timestampStream << std::put_time(
                &timeInfo,
                "%Y-%m-%d %H:%M:%S"
            );

            std::string timestamp =
                timestampStream.str();

            // Preserve the original TCP message format.
            std::string message =
                symbol + "," +
                std::to_string(price) + "," +
                timestamp + "\n";

            // Stop streaming if this client disconnects.
            if (!sendAll(clientSocket, message))
            {
                std::cout
                    << "Client disconnected. "
                    << "Stopping this stream.\n";

                break;
            }

            std::cout << "Sent: " << message;

            std::this_thread::sleep_for(
                std::chrono::seconds(1)
            );
        }

        // ---------------------------------------
        // 9. Clean up disconnected client
        // ---------------------------------------

        closesocket(clientSocket);

        std::cout
            << "Client socket closed.\n"
            << "Server remains active for reconnection.\n";

        // Outer loop returns to accept().
    }

    // ---------------------------------------
    // 10. Server cleanup
    // ---------------------------------------

    closesocket(serverSocket);
    WSACleanup();

    std::cout << "Server shutdown complete.\n";

    return 0;

#else

    std::cout << "This server is designed for Windows.\n";
    return 1;

#endif
}
