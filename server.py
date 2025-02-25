from flask import Flask, request, jsonify
from pymongo import MongoClient
from datetime import datetime

app = Flask(__name__)

# Konfigurasi MongoDB Atlas
MONGO_URI = "mongodb+srv://nasywaaworkingspace:1234@samsung.bwvlq.mongodb.net/?retryWrites=true&w=majority&appName=Samsung"
client = MongoClient(MONGO_URI)
db = client["sensor_database"]  # Ganti sesuai dengan nama database Anda
collection = db["sensor_data"]  # Ganti sesuai dengan koleksi yang diinginkan

@app.route('/api/sensor', methods=['POST'])
def receive_sensor_data():
    try:
        data = request.json
        if not data:
            return jsonify({"error": "No data received"}), 400

        print(f"ℹ️ Received data: {data}")

        # Tambahkan timestamp ke data
        data["timestamp"] = datetime.now()

        # Simpan data ke MongoDB
        result = collection.insert_one(data)
        print(f"✅ Data saved with ID: {result.inserted_id}")

        return jsonify({"message": "Data received and saved successfully"}), 201
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    
@app.route('/api/sensor', methods=['GET'])
def get_sensor_data():
    try:
        data = list(collection.find({}, {"_id": 0}))
        return jsonify(data)
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
