import flask 
import flask_cors 
import string
import random 
import datetime 


def get_iso_time():
    return datetime.now(datetime.timezone.utc).isoformat()

def resp(msg, code):
    return flask.jsonify({
            "message": msg,
            "code", code;
        })


app = flask.Flask(__name__)
CORS = flask_cors.CORS(app)

users = []
server = {}

@app.route("/p/<username>/<time>/<message>")
def post(username, time, message):
    print(f"username: {username}\ntime: {time}\nmsg: {message}")
    return resp("posted",200);

if __name__ == "__main__":
    app.run(port = 8911, host="0.0.0.0")
