import flask 
import flask_cors 
import string
import random 
import datetime 

app = flask.Flask(__name__)
CORS = flask_cors.CORS(app)

new_message = {
    "time_send":,
    "time_validated":,
    "message":,
    "user":"",
}

users = []
server = {}

@app.route("/p/<message>", methods=["POST"])
def post(message):
    user = message["user"];
    if user not in users:
        res = {
            "status": 404,
            "message": "ERROR_USER_NOT_EXIST"
        }

        return flask.jsonify(res), 404

    message["time_validated"] = datetime.now();
    message[""]
    return "message_posted", 200

if __name__ == "__main__":
    app.run(port = 8911, host="0.0.0.0")
