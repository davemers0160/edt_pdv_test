import platform
import os
import time

# this handles the *.dll/*.so file reading
from cffi import FFI

# numpy/opencv
import numpy as np
import cv2 as cv

# File dialog stuff
from PyQt5.QtWidgets import QFileDialog, QWidget, QApplication

import pandas as pd

# set up some global variables that will be used throughout the code
script_path = os.path.realpath(__file__)
image_path = os.path.dirname(os.path.dirname(script_path))

app = QApplication([""])

# variable to store the final feature space detections
fs = []
ffi = FFI()

os.chdir(os.path.dirname(__file__))
print(os.getcwd())

# -----------------------------------------------------------------------------
# This section allows cffi to get the info about the dll and the specialized data types
ffi.cdef('''

struct detection_struct{
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;
    char label[256];
    double score;
    unsigned int class_id;
};

void blob_detector(unsigned int img_w, unsigned int img_h, unsigned char* img_t, double threshold, unsigned int *num_dets, struct detection_struct** dets);

''')
# -----------------------------------------------------------------------------


# modify these to point to the right locations
if platform.system() == "Windows":
    libname = "blob_detector.dll"
    lib_location = "../../blob_detector/build/Release/" + libname
    # weights_file = "D:/Projects/dlib_object_detection/common/nets/td_v13a_020_020_100_90_HPC_final_net.dat"
elif platform.system() == "Linux":
    libname = "libblob_detector.so"
    home = os.path.expanduser('~')
    lib_location = home + "../../blob_detector/build/" + libname
    # weights_file = home + "/Projects/dlib_object_detection/common/nets/td_v13a_020_020_100_90_HPC_final_net.dat"
else:
    quit()

# read and write global
blob_det_lib = []
img_w = img_h = num_dets = 0
dets = []

def init_lib():
    global blob_det_lib, img_w, img_h, img_c, num_dets, dets

    blob_det_lib = ffi.dlopen(lib_location)

    # initialize the network with the weights file
    # det_win = ffi.new('struct window_struct**')
    # num_classes = ffi.new('unsigned int *')
    # num_win = ffi.new('unsigned int *')
    # obj_det_lib.init_net(weights_file.encode("utf-8"), num_classes, det_win, num_win)

    # detection_windows = pd.DataFrame(columns=["h", "w", "label"])
    # for idx in range(num_win[0]):
    #     detection_windows = detection_windows.append({"h": det_win[0][idx].h, "w": det_win[0][idx].w, "label": ffi.string(det_win[0][idx].label).decode("utf-8")}, ignore_index=True)

    # instantiate the run_net function
    img_w = ffi.new('unsigned int *')
    img_h = ffi.new('unsigned int *')
    num_dets = ffi.new('unsigned int *')
    dets = ffi.new('struct detection_struct**')


def get_input():
    global image_path

    image_name = QFileDialog.getOpenFileName(None, "Select a file",  image_path,
                                             "Image files (*.png *.jpg *.gif *.tiff *.tif);;All files (*.*)")
    # filename_div.text = "File name: " + image_name[0]
    if(image_name[0] == ""):
        return

    print("Processing File: ", image_name[0])
    # load in an image
    image_path = os.path.dirname(image_name[0])
    # retval, color_img = cv.imread(image_name[0])
    retval, color_img = cv.imreadmulti(image_name[0], flags=cv.IMREAD_GRAYSCALE | cv.IMREAD_ANYDEPTH)

    return color_img


# the main entry point into the code
# if __name__ == '__main__':

init_lib()

img_stack = get_input()

stack_size = len(img_stack)

threshold = 30.0
cv.namedWindow("test", cv.WINDOW_KEEPRATIO)

for img in img_stack:
    img_h, img_w = img.shape
    img_c = 1
    # img_d = img.depth()

    # normalize image and convert to uint8
    img = np.uint8(255 * ((img - img.min())/(img.max() - img.min())))

    # gray_img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)

    # run the image on network and get the results
    # void blob_detector(unsigned int img_w, unsigned int img_h, unsigned int img_c, unsigned char* img_t, double threshold, unsigned int *num_dets, struct detection_struct** dets);

    blob_det_lib.blob_detector(img_w, img_h, img.tobytes(), threshold, num_dets, dets)

    for idx in range(num_dets[0]):
        cv.rectangle(img, (dets[0][idx].x, dets[0][idx].y), (dets[0][idx].x+dets[0][idx].w, dets[0][idx].y+dets[0][idx].h), (255), 2)

    cv.imshow("test", img)
    cv.waitKey(50)

bp = 1


