from flask import Flask, jsonify, render_template, request
import webbrowser
import time
import random
import csv
from datetime import datetime

app = Flask(__name__)

@app.route('/stuff', methods = ['GET'])
def stuff():
    return jsonify(result=random.randint(0,10))

#@app.route('/')
#def index():
#    return render_template('index.html')

@app.route("/")
def hello():
    print(request.args.get("state"))
    with open('lock_status.csv', 'a') as file:
        writer = csv.writer(file)
        now = datetime.now()
        dt = now.strftime("%d/%m/%Y %H:%M:%S")
        writer.writerow([dt, request.args.get("state")])
    return "Hello, World!"

if __name__ == '__main__':
    app.run()