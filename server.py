import flask 
import flask_cors 
import string
import random 
from datetime import datetime, timezone 


def get_iso_time():
    return datetime.now(datetime.timezone.utc).isoformat()

def resp(msg, code):
    return flask.jsonify({
            "message": msg,
            "code": code
            })


app = flask.Flask(__name__)
CORS = flask_cors.CORS(app)

server = []

@app.route("/p/<username>/<time>/<message>")
def post(username, time, message):
    print(f"username: {username}\ntime: {time}\nmsg: {message}")
    
    message = {
            "username": username,
            "time_sent": time,
            "message": message,
            "time_valid": get_iso_time()
            }
    
    server.push(message);
    return resp("posted",200);

@app.route("/g")
def get_server_data():
    return flask.jsonify(server);

if __name__ == "__main__":
    app.run(port = 8911, host="0.0.0.0")
