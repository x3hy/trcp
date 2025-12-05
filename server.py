import flask 
import flask_cors 
import string
import random 
import datetime 


def get_iso_time():
    return datetime.now(datetime.timezone.utc).isoformat()


app = flask.Flask(__name__)
CORS = flask_cors.CORS(app)

users = []
server = {}

@app.route("/p/<username>/<time>/<message>")
def post(username, time, message):
    printf(f"username: {username}\ntime: {time}\nmsg: {message}")
    return "message_posted", 200

if __name__ == "__main__":
    app.run(port = 8911, host="0.0.0.0")
