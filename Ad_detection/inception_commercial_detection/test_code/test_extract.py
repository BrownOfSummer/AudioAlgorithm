import numpy as np
import os.path
from extractor import Extractor
from decode_video import get_video_info
import subprocess
import cv2
import time
import pickle

model = Extractor()

def read_frame_by_frame(fileloc, model):
    info = get_video_info(fileloc)
    width, height = int(info['width']), int(info['height'])
    duration = float(info['duration'])
    total_frames = int(info['frame_count'])
    fps = float(info['fps'])
    print("Video length: {}s;\nVideo Resolution: {} x {};".format(duration, height, width))
    print("Video frames: {};\nVideo fps: {};".format(total_frames, fps))

    command = ['ffmpeg',
                '-i', fileloc, 
                '-f', 'image2pipe', 
                '-pix_fmt', 'rgb24', 
                '-vcodec', 'rawvideo', '-']
    pipe = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE, bufsize=10**8 )

    sequences = []
    for _ in range(total_frames):
        # read height*width*3 bytes (= 1 frame)
        raw_image = pipe.stdout.read(height*width*3)
        # transform the byte read into a numpy array
        image = np.fromstring(raw_image, dtype='uint8')
        image = image.reshape((height,width,3))
        # throw away the data in the pipe's buffer.
        pipe.stdout.flush()
        #cv2.imshow('image', image)
        #cv2.waitKey(30)
        features = model.extract(image)
        sequences.append(features)
    return sequences, info

if __name__ == '__main__':
    video_path = '/Users/li_pengju/SomeDownload/Dataset/UCF101/UCF-101/Hammering/v_Hammering_g01_c01.avi'
    start = time.time()
    sequences, info = read_frame_by_frame(video_path, model)
    with open('tmp.vdna','wb') as f:
        pickle.dump(sequences, f)
    print("cost {}s".format(time.time() - start))
