
# Cài flask bằng code: pip install Flask

# Chạy 2 dòng sau để mở website: 
# .\venv\Scripts\activate
# waitress-serve --listen=0.0.0.0:5000 app:app

from flask import Flask, request, jsonify
import os  # Don't forget to include this line
from datetime import datetime

app = Flask(__name__)

@app.route("/")
def home():
    return "Hello world, from Flask!"

@app.route("/esp32-cam", methods=['GET'])
def esp32_cam():
    # Assuming you want to perform some actions before responding with "Done"
    # You can access POST data using request.json or request.form
    # For simplicity, we'll just return "Done" in the JSON response
    return jsonify({"message": "Done"})

@app.route("/sendpicture", methods=['POST'])
def receive_picture():
    try:
        # Get the image data from the request
        image_data = request.data

        # Create a folder named "pictures" if it doesn't exist
        pictures_folder = os.path.join(os.getcwd(), "pictures")
        os.makedirs(pictures_folder, exist_ok=True)

        # Generate a unique filename based on the current timestamp
        #timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        timestamp = datetime.now().strftime("thang%mngay%d_%Hg%Mp%Ss")
        filename = f"{timestamp}.jpg"

        # Save the image to the "pictures" folder
        image_path = os.path.join(pictures_folder, filename)
        with open(image_path, "wb") as file:
            file.write(image_data)

        print(f"Image saved to: {image_path}")

        return str("999-666"), 200

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"error": "Failed to process the image."}), 500



if __name__ == "__main__":
    app.run(debug=True)
