import json
import socket
import threading


def start_tcp_bridge(socketio):
    def handle_client(connection):
        buffer = ""

        try:
            with connection:
                while True:
                    data = connection.recv(4096)

                    if not data:
                        break

                    buffer += data.decode("utf-8")

                    while "\n" in buffer:
                        line, buffer = buffer.split("\n", 1)

                        try:
                            tick = json.loads(line)

                            print(f"Received market tick: {tick}", flush=True)

                            socketio.emit("price_update", tick)

                        except json.JSONDecodeError:
                            print("Invalid JSON received")

        except OSError as error:
            print(f"Bridge connection error: {error}")

    def run_server():
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server.bind(("127.0.0.1", 9090))
            server.listen(5)

            print("TCP bridge listening on port 9090")

            while True:
                connection, address = server.accept()
                print(f"C++ client connected: {address}")

                threading.Thread(
                    target=handle_client,
                    args=(connection,),
                    daemon=True
                ).start()

    threading.Thread(
        target=run_server,
        daemon=True
    ).start()
