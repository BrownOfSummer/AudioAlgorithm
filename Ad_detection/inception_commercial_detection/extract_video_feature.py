import numpy as np
import os.path
from extractor import Extractor
from helpers import get_video_info, generate_video_features
import subprocess
import cv2
import time
import pickle

model = Extractor()

if __name__ == '__main__':
    video_path = '/path/to/UCF101/UCF-101/Hammering/v_Hammering_g01_c01.avi'
    start = time.time()
    sequences, info = generate_video_features(video_path, model)
    with open('tmp.vdna','wb') as f:
        pickle.dump(sequences, f)
    print("cost {}s".format(time.time() - start))
