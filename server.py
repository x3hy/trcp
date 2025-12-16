import flask
import flask_cors
from datetime import datetime, timezone
from base64 import urlsafe_b64encode, urlsafe_b64decode


def uri_b64encode(s):
    return urlsafe_b64encode(s).strip('=')


def uri_b64decode(s):
    return urlsafe_b64decode(s + '=' * (4 - len(s) % 4))


def get_iso_time():
    return datetime.now(timezone.utc).isoformat()


def resp(msg, code):
    return flask.jsonify({
            "message": msg,
            "code": code
            })


app = flask.Flask(__name__)
CORS = flask_cors.CORS(app)
server = []

# http://example.com/p/my_us/ername/thetime/my message
@app.route("/post_msg/<username>/<time>/<message>")
def post(username, time, message):
    username = uri_b64decode(username).decode()
    time = uri_b64decode(time).decode()
    message = uri_b64decode(message).decode()

    print(f"username: {username}\ntime: {time}\nmsg: {message}")

    message = {
            "username": username,
            "time_sent": time,
            "message": message,
            "time_valid": get_iso_time()}

    server.append(message)
    return resp("message posted", 200)


@app.route("/get_msg")
def get_server_data():
    return flask.jsonify(server)


@app.route("/msg_count")
def get_server_n():
    print(server)
    return resp(len(server), 200)


if __name__ == "__main__":
    app.run(port=8911, host="0.0.0.0")
