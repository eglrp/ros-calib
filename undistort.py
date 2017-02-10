#!/usr/bin/env python

import argparse
import numpy
import cv2

class Undistorter:
    def set_alpha(self, a):
        ncm, _ = cv2.getOptimalNewCameraMatrix(self.intrinsics, self.distortion, self.size, a)
        for j in range(3):
            for i in range(3):
                self.P[j, i] = ncm[j, i]
        self.mapx, self.mapy = cv2.initUndistortRectifyMap(self.intrinsics, self.distortion, self.R, ncm, self.size,
                                                           cv2.CV_32FC1)

    def remap(self, src):
        return cv2.remap(src, self.mapx, self.mapy, cv2.INTER_LINEAR)

    def init(self, txt):
        i = 0
        def loadLine(line):
            return map(lambda x: float(x), line.split())
        def loadLines(lines):
            return numpy.array(map(loadLine, lines))
        while i < len(txt):
            if txt[i] == "width":
                i = i + 1
                w = int(txt[i])
            if txt[i] == "height":
                i = i + 1
                h = int(txt[i])
            if txt[i] == "distortion":
                i = i + 1
                self.distortion = numpy.array(loadLine(txt[i]))
            if txt[i] == "camera matrix":
                self.intrinsics = loadLines([txt[i + 1], txt[i + 2], txt[i + 3]])
                i = i + 3
            if txt[i] == "rectification":
                self.R = loadLines([txt[i + 1], txt[i + 2], txt[i + 3]])
                i = i + 3
            if txt[i] == "projection":
                self.P = loadLines([txt[i + 1], txt[i + 2], txt[i + 3]])
                i = i + 3
            i = i + 1
        self.size = (w, h)

def main():
    parser = argparse.ArgumentParser(description='Undistort camera images')
    parser.add_argument('--txt', type=file, required=True, help='text calibration file')
    parser.add_argument('-i', required=True, help='input image')
    parser.add_argument('-s', default=False, action='store_true', help='show image')
    parser.add_argument('-g', default=False, action='store_true', help='convert image to gray')
    parser.add_argument('-o', default="", help='output image')

    args = parser.parse_args()
    txt = args.txt.read().splitlines()
    und = Undistorter()
    und.init(txt)
    und.set_alpha(0)

    def process(image, args, idx, wait):
        if (args.g):
            image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        rect = und.remap(image)
        if args.o != "":
            cv2.imwrite(args.o % idx, rect)
        if args.s:
            cv2.imshow("Image", rect)
            return cv2.waitKey(wait)

    idx = 1
    if args.i.endswith(".png") or args.i.endswith(".jpg"):
        process(cv2.imread(args.i), args, idx, None)
    else:
        vc = cv2.VideoCapture(args.i)
        while True:
            ret_val, image = vc.read()
            if not ret_val:
                break
            if process(image, args, idx, 1) == 27:
                break
            idx = idx + 1

main()
