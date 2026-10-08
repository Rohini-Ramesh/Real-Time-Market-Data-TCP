# Real-Time Market Data TCP — MarketStream

A cross-platform, real-time market data streaming and visualization system built using **C++17, TCP sockets, Python, Flask-SocketIO, JavaScript, and Chart.js**.

The system simulates stock price updates on a Windows C++ server, transmits them over TCP to a Linux C++ client, processes market statistics, and displays the results in a live browser dashboard.

**Current Release:** v1.1  
**Status:** Implemented and tested  
**Market Data:** Simulated (not actual stock exchange prices)

## System Architecture

```text
Windows — C++ TCP Server
    |
    | Generates simulated stock prices
    | TCP Port 8080
    v
Chromebook Linux — C++ TCP Client
    |
    | Parses and validates price ticks
    | Calculates price changes and statistics
    | Converts processed ticks into JSON
    v
Python TCP Bridge — Port 9090
    |
    | Receives newline-delimited JSON
    | Emits Flask-SocketIO price_update events
    v
Flask Web Dashboard — Port 5000
    |
    | Live stock cards
    | Interactive Chart.js price charts
    | Market statistics
    | Feed connection monitoring
    v
Web Browser
```

## Features

### C++ TCP Market Data Server

- Generates simulated prices for AAPL, NVDA, MSFT, and GOOG.
- Sends a price tick approximately every second.
- Uses TCP sockets for reliable, ordered data transmission.
- Supports client disconnection and reconnection without restarting the server.
- Handles partial sends and socket transmission errors.
- Preserves simulated price state across client reconnections.

### C++ Market Data Client

- Connects to the Windows TCP server using a configurable IPv4 address.
- Receives and reconstructs newline-delimited market messages.
- Parses and validates price ticks.
- Calculates previous price, absolute change, and percentage change.
- Tracks minimum, maximum, average price, and tick counts.
- Converts processed ticks to structured JSON.
- Forwards JSON data to the Python dashboard bridge.

### Web Dashboard

- Live price cards for four simulated stocks.
- Price change indicators with positive and negative coloring.
- Interactive price-history charts using Chart.js.
- Stock selector for individual or combined chart views.
- Running market statistics.
- Live market-feed connection monitoring.
- Automatic dashboard status recovery when market ticks resume.

## Technology Stack

| Component | Technology |
|---|---|
| TCP Server | C++17, Winsock2, Windows |
| TCP Client | C++17, Linux POSIX sockets |
| Data Processing | C++ STL, nlohmann/json |
| TCP Bridge | Python, socket, threading |
| Web Backend | Flask, Flask-SocketIO |
| Frontend | HTML, CSS, JavaScript |
| Visualization | Chart.js |
| Version Control | Git, GitHub |

## Installation and Setup

### Prerequisites

- Windows computer with a C++ compiler and Winsock2 support.
- Linux environment with `g++`.
- Python 3 and `pip`.
- `nlohmann/json.hpp` available to the Linux compiler.
- Network connectivity between Windows and Linux.
- A modern web browser.

### 1. Clone the Repository

```bash
git clone https://github.com/Rohini-Ramesh/Real-Time-Market-Data-TCP.git
cd Real-Time-Market-Data-TCP
```

### 2. Compile and Run the Windows Server

From the repository root on Windows:

```powershell
g++ -std=c++17 server/src/server.cpp -o server.exe -lws2_32
.\server.exe
```

The server listens on TCP port `8080`.

Find the Windows machine's IPv4 address:

```powershell
ipconfig
```

### 3. Set Up the Python Dashboard on Linux

From the repository root:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Start the Flask dashboard and its TCP bridge:

```bash
python dashboard/app.py
```

The dashboard is available at:

`http://127.0.0.1:5000`

The Python bridge listens locally on TCP port `9090`.

### 4. Compile and Run the Linux Client

Open another Linux terminal in the repository root:

```bash
g++ -std=c++17 client/src/client.cpp -o /tmp/market_client
```

Start the client using your Windows server's current IPv4 address:

```bash
/tmp/market_client <WINDOWS_SERVER_IP>
```

For example:

```bash
/tmp/market_client 192.168.1.71
```

Replace the example address with the actual Windows IPv4 address.

### 5. Open the Dashboard

Open `http://127.0.0.1:5000` in your browser.

When the C++ client receives and forwards market ticks, the dashboard displays live stock prices, charts, and statistics.

## Connection Monitoring and Recovery

The dashboard distinguishes between the browser's connection to Flask and the arrival of market data.

- **Live Connected:** Market ticks are arriving.
- **Market Feed Disconnected:** No new tick has arrived within approximately five seconds.
- **Server Disconnected:** The browser has lost its Socket.IO connection to Flask.

The Windows C++ server also supports accepting a new TCP client after detecting a disconnected client.

**Current limitation:** The Linux C++ client does not automatically reconnect when its upstream TCP server disconnects. The client must be restarted manually.

## Testing and Debugging

The following scenarios were manually tested:

- TCP communication between Windows and Chromebook Linux.
- Streaming and parsing simulated market ticks.
- Forwarding processed JSON to the Python bridge.
- Live dashboard updates and interactive charts.
- Market-feed disconnection detection.
- Dashboard status recovery when streaming resumes.
- Reconnecting the C++ client without restarting the Windows server.
- IPv4 address configuration through command-line arguments.
- Invalid IPv4 address handling.

## Version History

### Version 1.0 — Original TCP Market Data System

Initial C++ TCP server and client implementation with simulated stock prices, message parsing, validation, and market statistics.

[View Version 1.0](https://github.com/Rohini-Ramesh/Real-Time-Market-Data-TCP/tree/v1.0)

### Version 1.1 — Web Dashboard and Reliability Improvements

Added Flask-SocketIO integration, a Python TCP bridge, live stock charts, market statistics, connection-status monitoring, TCP server reconnection handling, and configurable server IP addresses.

[View Version 1.1](https://github.com/Rohini-Ramesh/Real-Time-Market-Data-TCP/tree/v1.1)

## Future Roadmap

### Version 1.12 — Reliability Enhancements (Planned)

- Automatic Linux client reconnection.
- Python bridge connection recovery.
- Graceful server and client shutdown.
- Support for multiple simultaneous market-data clients.
- Improved heartbeat monitoring.
- Automated integration and fault-recovery tests.

### Version 2.0 — Real Market Data API Integration (Planned)

- Integrate a real stock market data API.
- Replace or supplement simulated prices with external market data.
- Handle API authentication and rate limits.
- Normalize incoming market-data messages.
- Add data-source and market-data freshness indicators.
- Preserve simulated-data mode for offline demonstrations.

## Engineering Concepts Demonstrated

- TCP/IP socket programming
- Cross-platform C++ development
- Network protocol design and message framing
- Stream parsing and validation
- Stateful statistical processing
- JSON serialization
- Python multithreading
- Real-time browser updates using Socket.IO
- Fault detection and connection recovery
- Git branching, pull requests, and versioned releases

## Disclaimer

This project is intended for software engineering education, technical demonstrations, and interview preparation. All prices in Versions 1.0 and 1.1 are simulated and should not be used for financial decisions.
