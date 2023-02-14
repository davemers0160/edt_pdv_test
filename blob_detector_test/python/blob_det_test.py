import platform
import os
# this handles the *.dll/*.so file reading
import time

from cffi import FFI
# numpy/opencv
import numpy as np
import cv2 as cv
# File dialog stuff
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QFileDialog, QWidget, QApplication

import pandas as pd
import bokeh
from bokeh.io import curdoc
from bokeh.models import ColumnDataSource, HoverTool, Button, Div
from bokeh.plotting import figure, show
from bokeh.layouts import column, row, Spacer

# set up some global variables that will be used throughout the code
script_path = os.path.realpath(__file__)
image_path = os.path.dirname(os.path.dirname(script_path))
# update_time = 100
# num_det_wins = 0
# detection_windows = []
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

void blob_detector(unsigned int img_w, unsigned int img_h, unsigned int img_c, unsigned char* img_t, double threshold, unsigned int *num_dets, struct detection_struct** dets);

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
obj_det_lib = []
x_r = y_r = min_img = max_img = 0

# def init_lib():
#     global obj_det_lib, ls_01, ld_01, num_classes, detection_windows, \
#         t_nr, t_nc, tiled_img, det_img, num_dets, dets, dc
#
#     obj_det_lib = ffi.dlopen(lib_location)
#
#     # initialize the network with the weights file
#     # void init_net(const char *net_name, unsigned int *num_classes, window** det_win, unsigned int *num_win)
#     det_win = ffi.new('struct window_struct**')
#     num_classes = ffi.new('unsigned int *')
#     num_win = ffi.new('unsigned int *')
#     obj_det_lib.init_net(weights_file.encode("utf-8"), num_classes, det_win, num_win)
#
#     detection_windows = pd.DataFrame(columns=["h", "w", "label"])
#     for idx in range(num_win[0]):
#         detection_windows = detection_windows.append({"h": det_win[0][idx].h, "w": det_win[0][idx].w, "label": ffi.string(det_win[0][idx].label).decode("utf-8")}, ignore_index=True)
#
#     # instantiate the run_net function
#     # void run_net(unsigned char* image, unsigned int nr, unsigned int nc, unsigned char** tiled_img, unsigned int *t_nr, unsigned int *t_nc, unsigned char** det_img, unsigned int *num_dets, struct detection_struct** dets);
#     tiled_img = ffi.new('unsigned char**')
#     t_nr = ffi.new('unsigned int *')
#     t_nc = ffi.new('unsigned int *')
#     #det_img = ffi.new('unsigned char**')
#     num_dets = ffi.new('unsigned int *')
#     dets = ffi.new('struct detection_struct**')
#     #dc = ffi.new('struct detection_center**')
#
#     # instantiate the get_layer_01 function
#     # void get_layer_01(struct layer_struct *data, const float** data_params);
#     ls_01 = ffi.new('struct layer_struct*')
#     ld_01 = ffi.new('float**')


def get_input():
    global detection_windows, image_path, rgba_img

    image_name = QFileDialog.getOpenFileName(None, "Select a file",  image_path,
                                             "Image files (*.png *.jpg *.gif *.tiff *.tif);;All files (*.*)")
    # filename_div.text = "File name: " + image_name[0]
    if(image_name[0] == ""):
        return

    print("Processing File: ", image_name[0])
    # load in an image
    # image_path = os.path.dirname(image_name[0])
    # retval, color_img = cv.imread(image_name[0])
    retval, color_img = cv.imreadmulti(image_name[0], flags=cv.IMREAD_ANYCOLOR | cv.IMREAD_ANYDEPTH)

    return color_img


def run_detection(color_img):
    global p1, obj_det_lib, num_dets, dets, det_img, rgba_img, img_nr, img_nc, dc, tiled_img, t_nr, t_nc

    gray_img = cv.cvtColor(color_img, cv.COLOR_BGR2GRAY)
    color_img = cv.cvtColor(color_img, cv.COLOR_BGR2RGB)
    img_nr = color_img.shape[0]
    img_nc = color_img.shape[1]

    # run the image on network and get the results
    obj_det_lib.get_detections(gray_img.tobytes(), img_nr, img_nc, num_dets, dets)
    #obj_det_lib.run_net(gray_img.tobytes(), img_nr, img_nc, det_img, num_dets, dets)
    obj_det_lib.get_pyramid_tiled_input(gray_img.tobytes(), img_nr, img_nc, tiled_img, t_nr, t_nc)


def update_plots():
    global ls_01, ld_01, detection_windows, t_nr, t_nc, tiled_img, num_dets, dets, \
        results_div, rgba_img, img_nr, img_nc, p1, p2, ti, fs, dc

    detections = pd.DataFrame(columns=["x", "y", "h", "w", "label"])
    det_results = "<font size='3'>"
    dets_text = "{:04d}".format(0) + ","
    for idx in range(num_dets[0]):
        detections = detections.append({"x": dets[0][idx].x, "y": dets[0][idx].y, "h": dets[0][idx].h, "w": dets[0][idx].w, "label": ffi.string(dets[0][idx].label).decode("utf-8")}, ignore_index=True)
        det_results += str(detections["x"][idx]) + ", " + str(detections["y"][idx]) + ", " + str(detections["h"][idx]) + ", " + str(detections["w"][idx]) + ", " + detections["label"][idx] + "<br>"
        dets_text += "{" + str(detections["x"][idx]) + ", " + str(detections["y"][idx]) + ", " + str(detections["h"][idx]) + ", " + str(detections["w"][idx]) + ", " + detections["label"][idx] + "},"

    det_results += "</font>"
    results_div.text = det_results  # "<font size='4'>" + str(index) + ": " + file_path[0] + "</font>"

    print(dets_text[0:-1])

    det_img_view = np.copy(rgba_img)
    for idx in range(num_dets[0]):
        cv.rectangle(det_img_view, (dets[0][idx].x, dets[0][idx].y), (dets[0][idx].x+dets[0][idx].w, dets[0][idx].y+dets[0][idx].h), (255, 0, 0, 255), 2)

    tiled_img_alpha = np.full((t_nr[0], t_nc[0]), 255, dtype=np.uint8)
    tiled_img_view = np.dstack([np.reshape(np.frombuffer(ffi.buffer(tiled_img[0], t_nr[0] * t_nc[0] * 3), dtype=np.uint8), [t_nr[0], t_nc[0], 3]),
                                tiled_img_alpha])

    p2.image_rgba(image=[np.flipud(det_img_view)], x=0, y=0, dw=400, dh=400)
    ti.image_rgba(image=[np.flipud(tiled_img_view)], x=0, y=0, dw=400, dh=400)

    # start to create the source data based on the static/know inputs
    #source.data = {'input_img': [np.flipud(rgba_img)], 'det_view': [np.flipud(det_img_view)],
    #               'tiled_img': [np.flipud(tiled_img_view)]}

    # get the Layer 01 data and shape it correctly
    obj_det_lib.get_layer_01(ls_01, ld_01)
    l01_data = np.frombuffer(ffi.buffer(ld_01[0], ls_01.size * 4), dtype=np.float32)
    l01_min = np.amin(l01_data)
    l01_max = np.amax(l01_data)
    img_length = ls_01.nr * ls_01.nc
    # l01_all = np.reshape(l01_data, [ls_01.nr, ls_01.nc, ls_01.k], order='C')

    # l01_01 = np.reshape(l01_data[0:img_length], [ls_01.nr, ls_01.nc])
    # l01_jet = jet_color(l01_01, l01_min, l01_max)
    # l01_jet = cv.resize(l01_jet, (10 * ls_01.nr, 10 * ls_01.nc), interpolation=cv.INTER_NEAREST)
    # cv.imshow("test", l01_jet)
    # cv.waitKey(-1)

    l01_list = []
    for idx in range(ls_01.k):
        # sd_key = "l01_img_" + "{:04d}".format(idx)
        # fl_data = "l01_imgf_" + "{:04d}".format(idx)
        s1 = idx*img_length
        s2 = (idx+1)*img_length
        l01_list.append(np.reshape(l01_data[s1:s2], [ls_01.nr, ls_01.nc]))
        # l01_jet = jet_color(l01_list[idx], l01_min, l01_max)
        l01_jet = jet_color(l01_list[idx], -2.0, 0.0)
        fs[idx].image_rgba(image=[np.flipud(l01_jet)], x=0, y=0, dw=400, dh=400)
        # source.data[sd_key] = [np.flipud(l01_jet)]
        # source.data[fl_data] = [l01_list[idx]]
        # fs[idx].tools[0].tooltips = {"Value": "$l01_list[idx]"}

    bp = 1


# the main entry point into the code
# if __name__ == '__main__':

img_stack = get_input()

stack_size = len(img_stack)

for img in img_stack:







bp = 1


