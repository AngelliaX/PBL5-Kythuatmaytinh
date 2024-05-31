from ultralytics import YOLO
from PIL import Image

model = YOLO('../runs/detect/train3/weights/best.pt')

results = model('../test_images_steelPipe/ongthep12.jpg')

for r in results:
    print(r.boxes)
    im_array = r.plot()
    im = Image.fromarray(im_array[..., ::-1])
    im.show()
    im.save('../result_images_steelPipe/resultSteelPipe5.jpg')

print("Done running");