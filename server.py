
#######################################
# Tạo thư mục flask ở ngoài cùng (ngang hàng với folder executable)
# add file server.py vào folder flask, khi chạy thì ảnh được nhận từ esp32-cam sẽ tự lưu vào folder ./pictures
# chỉ cần thêm 1 thư viện: pip install flask
# dùng "python server.py" để chạy
#######################################

# import flask
from flask import Flask, request, jsonify
import os
from datetime import datetime

# import yolov8
from ultralytics import YOLO
from PIL import Image

from ultralytics import YOLO
from PIL import Image

app = Flask(__name__)

@app.route("/")
def home():
    return "Hello world, from Flask!"

@app.route('/predict', methods=['POST'] )
def predict_yolov8():
    try:
        ##########################################
        # Code lưu ảnh nhận được vào ./pictures
        ##########################################

        image_data = request.data
        pictures_folder = os.path.join(os.getcwd(), "pictures")
        os.makedirs(pictures_folder, exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        filename = f"{timestamp}.jpg"
        image_path = os.path.join(pictures_folder, filename)
        with open(image_path, "wb") as file:
            file.write(image_data)

        ##########################################
        # bắt đầu AI nhận diện
        ##########################################
        model = YOLO('../runs/detect/train3/weights/best.pt')
        results = model(os.path.join('./pictures', filename));
        


        ##########################################
        # điền kết quả nhận diện ở dưới đây:
        ##########################################
        finalResultPredicted = 69;



        ##########################################
        # xong
        ##########################################
        return str(finalResultPredicted), 200

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"error": "Failed to process the image."}), 500


# Start Backend
if __name__ == '__main__':
    app.run(host='0.0.0.0', port='5000')