
const socket = io();
const marketData = document.getElementById("market-data");
const status = document.getElementById("status");

const stockRows = {};
// Market feed monitoring
const FEED_TIMEOUT_MS = 5000;

let lastTickTime = null;

function updateConnectionStatus(message, connected) {
    status.textContent = "● " + message;
    status.classList.toggle("connected", connected);
}

// Check the market feed every second
setInterval(() => {
    if (!socket.connected) {
        updateConnectionStatus("Server Disconnected", false);
        return;
    }

    if (
        lastTickTime === null ||
        Date.now() - lastTickTime > FEED_TIMEOUT_MS
    ) {
        updateConnectionStatus("Market Feed Disconnected", false);
    }
}, 1000);

socket.on("connect", () => {
    lastTickTime = null;
    updateConnectionStatus("Waiting for Market Data", false);
});

socket.on("disconnect", () => {
    lastTickTime = null;
    updateConnectionStatus("Server Disconnected", false);
});

socket.on("price_update", (tick) => {
    if (!socket.connected) {
        return;
    }

    lastTickTime = Date.now();

    updateConnectionStatus("Live Connected", true);

    const symbol = tick.symbol;

    updatePriceChart(tick);
    updateStatistics(tick);

const priceElement = document.getElementById("price-" + symbol);
const changeElement = document.getElementById("change-" + symbol);

if (priceElement && changeElement) {
    const price = Number(tick.price);
    const change = Number(tick.change);
    const percent = Number(tick.changePercent);

    priceElement.textContent = "$" + price.toFixed(2);

    if (tick.hasPreviousPrice) {
        const sign = change > 0 ? "+" : "";

        changeElement.textContent =
            sign + change.toFixed(2) +
            " (" + sign + percent.toFixed(2) + "%)";

        changeElement.className =
            change >= 0 ? "positive" : "negative";
    } else {
        changeElement.textContent = "Waiting for previous price";
        changeElement.className = "";
    }
}


    if (!["AAPL", "NVDA", "MSFT", "GOOG"].includes(symbol)) {
        return;
    }

    if (!stockRows[symbol]) {
        const row = document.createElement("tr");

        for (let i = 0; i < 5; i++) {
            row.appendChild(document.createElement("td"));
        }

        marketData.appendChild(row);
        stockRows[symbol] = row;
    }

    const cells = stockRows[symbol].children;
    const change = Number(tick.change);
    const percent = Number(tick.changePercent);

    cells[0].textContent = symbol;
    cells[1].textContent = "$" + Number(tick.price).toFixed(2);
    cells[2].textContent =
        tick.hasPreviousPrice ? change.toFixed(2) : "N/A";
    cells[3].textContent =
        tick.hasPreviousPrice ? percent.toFixed(2) + "%" : "N/A";
    cells[4].textContent = tick.timestamp;

    const color = change >= 0 ? "green" : "red";
    cells[2].style.color = color;
    cells[3].style.color = color;
});

const chartSymbols = ["AAPL", "NVDA", "MSFT", "GOOG"];

const chartColors = {
    AAPL: "#38bdf8",
    NVDA: "#4ade80",
    MSFT: "#fbbf24",
    GOOG: "#fb7185"
};

const chartData = {};
const maxDataPoints = 30;

chartSymbols.forEach(symbol => {
    chartData[symbol] = [];
});

const priceChart = new Chart(
    document.getElementById("priceChart"),
    {
        type: "line",
        data: {
            datasets: chartSymbols.map(symbol => ({
                label: symbol,
                data: chartData[symbol],
                borderColor: chartColors[symbol],
                backgroundColor: chartColors[symbol],
                borderWidth: 2,
                tension: 0.3,
                pointRadius: 2,
                parsing: false
            }))
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: false,
            scales: {
                x: {
                    type: "linear",
                    title: {
                        display: true,
                        text: "Elapsed Time (seconds)",
                        color: "#94a3b8"
                    },
                    ticks: { color: "#94a3b8" },
                    grid: { color: "#334155" }
                },
                y: {
                    title: {
                        display: true,
                        text: "Stock Price (USD)",
                        color: "#94a3b8"
                    },
                    ticks: { color: "#94a3b8" },
                    grid: { color: "#334155" }
                }
            },
            plugins: {
                legend: {
                    labels: { color: "#e2e8f0" }
                }
            }
        }
    }
);

const chartStartTime = Date.now();

function updatePriceChart(tick) {
    const symbol = tick.symbol;
    const price = Number(tick.price);

    if (!chartSymbols.includes(symbol) ||
        !Number.isFinite(price)) {
        return;
    }

    const elapsedSeconds =
        (Date.now() - chartStartTime) / 1000;

    chartData[symbol].push({
        x: elapsedSeconds,
        y: price
    });

    if (chartData[symbol].length > maxDataPoints) {
        chartData[symbol].shift();
    }

    priceChart.update("none");
}

const stockSelector = document.getElementById("stockSelector");

stockSelector.addEventListener("change", () => {
    const selectedStock = stockSelector.value;

    priceChart.data.datasets.forEach(dataset => {
        dataset.hidden =
            selectedStock !== "ALL" &&
            dataset.label !== selectedStock;
    });

    priceChart.update();
if (stockSelector.value !== "ALL" && latestTicks[stockSelector.value]) {
    updateStatistics(latestTicks[stockSelector.value]);
}
});

const latestTicks = {};

function updateStatistics(tick) {
    latestTicks[tick.symbol] = tick;

    const selected = stockSelector.value;

    const activeSymbol =
        selected === "ALL" ? tick.symbol : selected;

    const activeTick = latestTicks[activeSymbol];

    if (!activeTick) {
        return;
    }

    document.getElementById("stat-average").textContent =
        "$" + Number(activeTick.averagePrice).toFixed(2);

    document.getElementById("stat-min").textContent =
        "$" + Number(activeTick.minPrice).toFixed(2);

    document.getElementById("stat-max").textContent =
        "$" + Number(activeTick.maxPrice).toFixed(2);

    document.getElementById("stat-count").textContent =
        activeTick.count;

    document.getElementById("stat-total").textContent =
        tick.totalTicks;

    document.getElementById("stat-invalid").textContent =
        tick.invalidTicks;

    document.getElementById("stat-symbol").textContent =
    selected === "ALL"
        ? "Showing latest updated stock: " + activeSymbol
        : "Statistics for " + activeSymbol;
}
