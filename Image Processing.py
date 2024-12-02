import urllib.request
import numpy as np
import cv2

def cam():
    url='http://172.20.10.3/1600x1200.jpg'

    imgResp = urllib.request.urlopen(url)
    
    imgNp = np.array(bytearray(imgResp.read()),dtype=np.uint8)
    image1 = cv2.imdecode(imgNp,-1)
    return image1

img=cam()

# cv2.imshow('Original Image', img)
# cv2.waitKey(0)

hsv_img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

lower_white = (0, 0, 200)
upper_white = (180, 40, 255)
mask_white = cv2.inRange(hsv_img, lower_white, upper_white)

lower_gray = (0, 0, 80)
upper_gray = (180, 40, 200)
mask_gray = cv2.inRange(hsv_img, lower_gray, upper_gray)

mask = cv2.bitwise_or(mask_white, mask_gray)

height, width = img.shape[:2]

h_step = height // 2
w_step = width // 3

for i in range(2):
    for j in range(3):
        y_start = i * h_step
        y_end = (i + 1) * h_step
        x_start = j * w_step
        x_end = (j + 1) * w_step

        sub_mask = mask[y_start:y_end, x_start:x_end]

        total_pixels = sub_mask.size
        detected_pixels = cv2.countNonZero(sub_mask)
        percentage = (detected_pixels / total_pixels) * 100

        print(f"Section ({i+1},{j+1}): {percentage:.2f}%")

color_image = cv2.bitwise_and(img, img, mask=mask)

# cv2.imshow('Filtered Image', color_image)
# cv2.waitKey(0)

cv2.destroyAllWindows()
