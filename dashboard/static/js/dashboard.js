
const socket = io();
const marketData = document.getElementById("market-data");
const status = document.getElementById("status");

const stockRows = {};

socket.on("connect", () => {
    status.textContent = "Connected to dashboard server";
});

socket.on("disconnect", () => {
    status.textContent = "Dashboard connection lost";
});

socket.on("price_update", (tick) => {
    const symbol = tick.symbol;

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
