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

int main()
{
#ifdef _WIN32

    // Initialize Windows socket system
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "WSA Startup failed\n";
        return 1;
    }

#endif

    // ---------------------------------------
    // 1. Create server socket
    // ---------------------------------------

    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (serverSocket == INVALID_SOCKET)
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
    serverAddress.sin_addr.s_addr = INADDR_ANY;


    // ---------------------------------------
    // 3. Bind socket
    // ---------------------------------------

    if (bind(
        serverSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == SOCKET_ERROR)
    {
        std::cout << "Bind failed\n";

#ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
#endif

        return 1;
    }

    std::cout << "Server bound to port "
              << PORT << "\n";


    // ---------------------------------------
    // 4. Listen
    // ---------------------------------------

    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        std::cout << "Listen failed\n";

#ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
#endif

        return 1;
    }

    std::cout << "Waiting for client connection...\n";


    // ---------------------------------------
    // 5. Accept Chromebook client
    // ---------------------------------------

    SOCKET clientSocket = accept(
        serverSocket,
        nullptr,
        nullptr
    );

    if (clientSocket == INVALID_SOCKET)
    {
        std::cout << "Accept failed\n";

#ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
#endif

        return 1;
    }

    std::cout << "Client connected successfully!\n";


    // ---------------------------------------
    // 6. Generate mock price ticks
    // ---------------------------------------

    std::vector<std::string> symbols =
    {
        "AAPL",
        "NVDA",
        "MSFT",
        "GOOG"
    };


    std::random_device rd;
    std::mt19937 generator(rd());

    std::uniform_real_distribution<double> changeDistribution(
        -1.00,
        1.00
    );

    std::map<std::string, double> prices =
    {
        {"AAPL", 215.00},
        {"NVDA", 187.00},
        {"MSFT", 512.00},
        {"GOOG", 201.00}
    };


    // ---------------------------------------
    // 7. Send 10 price ticks
    // ---------------------------------------

    int symbolIndex = 0;

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

        std::string message =
            symbol + "," +
            std::to_string(price) + "," +
            timestamp +
            "\n";


        send(
            clientSocket,
            message.c_str(),
            static_cast<int>(message.size()),
            0
        );


        std::cout
            << "Sent: "
            << message;


        std::this_thread::sleep_for(
            std::chrono::seconds(1)
        );
    }


    std::cout << "\nFinished sending price ticks.\n";


    // ---------------------------------------
    // 8. Close sockets
    // ---------------------------------------

#ifdef _WIN32

    closesocket(clientSocket);
    closesocket(serverSocket);

    WSACleanup();

#endif

    return 0;
}