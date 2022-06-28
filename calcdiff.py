import numpy as np
import cv2
import argparse

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--i1", type=str)
    parser.add_argument("--i2", type=str)
    
    return parser.parse_args()

def main():
    args=get_args()
    path1=args.i1
    path2=args.i2
    img1=cv2.imread(path1)
    img2=cv2.imread(path2)
    img1=img1.astype(np.int)
    img2=img2.astype(np.int)
    # print(img1[-2][-2])
    # print(img2[-2][-2])
    # print(img1[-1][-1])
    # print(img1[0][-1])
    # print(img1[-1][0])
    # print(img1[0][0])
    # print(img2[-1][-1])
    # print(img2[0][-1])
    # print(img2[-1][0])
    # print(img2[0][0])
    cnt=0
    h,w=img1.shape[0],img1.shape[1]
    for i in range(h):
        for j in range(w):
            s=np.sum(abs(img1[i][j]-img2[i][j]))
            if(s>0):
                print(s)
                cnt+=1
    # for i,j in zip(img1,img2):
    print(cnt)
    print(cnt/(img1.shape[0]*img1.shape[1]))
    #     print(np.sum(abs(j-i)))
    # print(img1.shape)
    # print(img2.shape)
    # print(img1-img2)
    return 0
if __name__ == '__main__':
    main()