
#######################################
# Tạo thư mục tên "flask" ở ngoài cùng (ngang hàng với folder executable)
# add file server.py vào folder flask, khi chạy thì ảnh được nhận từ esp32-cam sẽ tự lưu vào folder ./pictures
#######################################

# D:\Software\Python\Python312\Scripts\pip.exe install flask
# D:\Software\Python\Python312\Scripts\pip.exe install torch
# D:\Software\Python\Python312\Scripts\pip.exe install yolov8
# D:\Software\Python\Python312\Scripts\pip.exe install ultralytics
# D:\Software\Python\Python312\Scripts\pip.exe install numpy
# py server.py

#######################################

# cd C:\Users\namxm\OneDrive\Máy tính\Ki2Nam3\PBL5\PBL5 - AI\flask
# py server.py

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

import numpy as np

app = Flask(__name__)

@app.route("/")
def home():
    return "Hello world, from Flask!"


@app.route('/sendpicture', methods=['POST'] )
def predict_yolov8():
    try:
        print("\n\n\n======================================")
        print(datetime.now().time()," Processing...")
        ##########################################
        # Code lưu ảnh nhận được vào ./pictures
        ##########################################

        image_data = request.data
        pictures_folder = os.path.join(os.getcwd(), "pictures")
        os.makedirs(pictures_folder, exist_ok=True)
        timestamp = datetime.now().strftime("%m-%d-%Hh%Mp")
        filename = f"{timestamp}.jpg"
        image_path = os.path.join(pictures_folder, filename)
        with open(image_path, "wb") as file:
            file.write(image_data)
        
        print(datetime.now().time()," Image received...")

        ##########################################
        # bắt đầu AI nhận diện
        ##########################################
        
        model = YOLO('../runs/detect/train3/weights/best.pt')
        results = model(os.path.join('./pictures', filename));

        # Count number of pipe
        boxDetected = int( (results[0].boxes.cpu().numpy().xywh.size) /4 )
        confidence = (100 
              if results[0].boxes.conf.cpu().numpy().size == 0 
                 or np.isnan(np.mean(results[0].boxes.conf.cpu().numpy())) 
              else round(np.mean(results[0].boxes.conf.cpu().numpy()) * 100))
        
        print('\n');
        print(datetime.now().time());
        print('👉👉👉 NUMBER OF PIPE: ', boxDetected)
        print('👉👉👉 OVERALL CONF: ', confidence, '%')

        ##########################################
        # điền kết quả nhận diện ở dưới đây:
        ##########################################
        finalResultPredicted = str(boxDetected) + "-" + str(confidence);


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