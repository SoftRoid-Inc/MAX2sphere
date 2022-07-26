import numpy 
import cv2

img1=cv2.imread("../../8/130/2022/04/27/02:34:31.000000/prepro/images/er/er_frmame_001111.jpg")
img2=cv2.imread("../../8/130/2022/04/27/02:34:31.000000/prepro/images/er2/er_frnmame_image_001111_sphere.jpg")
print((img1-img2).sum())