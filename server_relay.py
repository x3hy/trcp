"""
This file is ONLY a relay, it will ONLY
store and send messages from one place to
another. This will be re-implemented in C.
"""
from flask import Flask, Response, jsonify, request
from flask_cors import CORS

# Flask setup
app = Flask(__name__)
CORS = CORS(app)
db = {}

# Basic POST rule
@app.route("/post", methods=['POST'])
def POST():
    gid = request.args["id"];
    msg  = request.args["content"];
    if not db[gid]:
        # Create branch
        db[gid] = []

    # Append message
    db[uuid].append(msg)
    return 

# Even more basic GET rule
@app.route("/get", methods=['GET'])
def GET():
    gid = request.args["id"]
    return db[gid].join["\n"];

if __name__ == '__main__':
    app.run();
