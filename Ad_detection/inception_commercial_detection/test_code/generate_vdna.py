# -*- coding:utf-8 -*-
from __future__ import print_function
import os
import cv2
from tqdm import tqdm

def GetVideoInfo(cap, video_path):
    assert os.path.exists(video_path)
    video_height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
    video_width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
    total_frames = cap.get(cv2.CAP_PROP_FRAME_COUNT)
    frame_rate = cap.get(cv2.CAP_PROP_FPS)
    return video_height, video_width, total_frames, frame_rate

def decode_video(video_path):
    assert os.path.exists(video_path)
    cap = cv2.VideoCapture(video_path)
    if cap.isOpened() is False:
        print('Open error\n')
        return None
    height, width, total_frames, frame_rate = GetVideoInfo(cap, video_path)
    print("Resolution {} x {}".format(height, width))
    print("Video frames: {};\nframe_rate: {};\nframe_length: {} s;".format(
        total_frames, frame_rate, float(total_frames)/float(frame_rate) ))

    pbar = tqdm(total=total_frames)
    for _ in range(int(total_frames)):
        cap.grab()
        value, frame = cap.retrieve()
        print(value, type(value))
        print(frame, type(frame))
        break
        cv2.imshow('video',frame)
        cv2.waitKey(30)
        pbar.update(1)
    pbar.close()

if __name__ == '__main__':
    video_path = '/Users/li_pengju/SomeDownload/Dataset/uray-4v/a/10/datadir0/a-10.mp4'
    #video_path = '/Users/li_pengju/SomeDownload/Dataset/UCF101/UCF-101/Hammering/v_Hammering_g01_c01.avi'
    decode_video(video_path)
