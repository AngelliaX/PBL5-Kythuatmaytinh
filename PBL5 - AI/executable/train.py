from ultralytics import YOLO
# import os
#
# os.environ["PYTORCH_CUDA_ALLOC_CONF"] = "expandable_segments:True"

model = YOLO('../models/yolov8n.pt')

if __name__ == '__main__':
    results = model.train(data='D:/PyCharmProjects/PBL5/yaml/steel_pipe_data.yaml', epochs=50, device='0')

