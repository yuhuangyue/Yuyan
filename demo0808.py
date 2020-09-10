# live_scene_and_gaze.py : A demo for video streaming and synchronized gaze
#
# Copyright (C) 2018  Davide De Tommaso
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>

import cv2
import numpy as np
from numpy.ctypeslib import ndpointer
import ctypes
from ctypes import *
import marker_detect_14

if hasattr(__builtins__, 'raw_input'):
      input=raw_input

from tobiiglassesctrl.controller import TobiiGlassesController


class handle_t(Structure):
  pass

address = "192.168.71.50"
#address = "fe80::76fe:48ff:ff00:ff00"
tobiiglasses = TobiiGlassesController(address, video_scene=True)
project_id = tobiiglasses.create_project("Test live_scene_and_gaze.py")
participant_id = tobiiglasses.create_participant(project_id, "participant_test")
calibration_id = tobiiglasses.create_calibration(project_id, participant_id)
input("Put the calibration marker in front of the user, then press enter to calibrate")
tobiiglasses.start_calibration(calibration_id)
res = tobiiglasses.wait_until_calibration_is_done(calibration_id)
ctypes.WinDLL('D:\\5Yuyan\\TestReport\\Worker\\x64\\Debug\\opencv_world347d.dll')
# ctypes.WinDLL('D:\\5Yuyan\\TestReport\\new_code\\code\\YuYan\\x64\\Debug\\opencv_ffmpeg347_64.dll')
dll = ctypes.WinDLL('D:\\5Yuyan\\TestReport\\Worker\\x64\\Debug\\Worker.dll')
dll.Worker_new.restype = POINTER(handle_t)
obj = dll.Worker_new()


class Point(Structure):
    _fields_ = ("num", c_int), ("x", c_float), ("y", c_float)

if res is False:
	print("Calibration failed!")
	exit(1)


tobiiglasses.start_streaming()
video_freq = tobiiglasses.get_video_freq()
frame_duration = 1000.0/float(video_freq) #frame duration in 40 ms

input("Press ENTER to start the video scene")
#address = "fe80::76fe:48ff:fe35:31f6"

dll.Worker_new.restype = POINTER(handle_t)
# obj = dll.Worker_new()
cap = cv2.VideoCapture("rtsp://%s:8554/live/scene" % address)
import datetime
while(1):

    ret, frame = cap.read()
    height, width = frame.shape[:2]

    data_gp = tobiiglasses.get_data()['gp']
    data_pts = tobiiglasses.get_data()['pts']
    data_gaze3d = tobiiglasses.get_data()['gp3']

    # 转换数据
    frame_data = frame.ctypes.data_as(ctypes.c_char_p)
    dll.Worker_Load_Image(obj, height, width, frame_data)
    #dll.show_image(height, width, frame_data)
    offset = data_gp['ts'] / 1000000.0 - data_pts['ts'] / 1000000.0
    if offset > 0.0 and offset <= frame_duration:
        # 获取numpy对象的数据指针
        x = round(data_gp['gp'][0], 2)
        y = round(data_gp['gp'][1], 2)
        z = round(data_gaze3d['gp3'][2], 2)

        x = c_float(x)
        y = c_float(y)
        z = c_float(z)

        dll.Worker_Valid_Gaze_Data(obj, x, y, z)

    if ret == False:
        break
