from flask import Flask, render_template
from flask_socketio import SocketIO
from bridge.tcp_bridge import start_tcp_bridge

app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static"
)

socketio = SocketIO(app, async_mode="threading")


@app.route("/")
def index():
    return render_template("index.html")


if __name__ == "__main__":
    start_tcp_bridge(socketio)

    print("Starting Market Data Dashboard...")
    socketio.run(
        app,
        host="127.0.0.1",
        port=5000,
        debug=True,
        use_reloader=False,
        allow_unsafe_werkzeug=True
    )
