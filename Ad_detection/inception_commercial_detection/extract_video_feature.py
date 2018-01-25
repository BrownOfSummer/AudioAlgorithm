# -*- using:utf-8 -*-
import numpy as np
import os
from extractor import Extractor
from helpers import get_video_info, generate_video_features
import subprocess
import time
import pickle
import glob

from tqdm import tqdm

model = Extractor()

ROOT_DIR = '../../data' # path to UCF-101
VIDEO_FEATURE_DIR = './VIDEO_FEATURE_DIR' # path to save video features
VIDEO_FILES = './video_files.txt'

def prepare_vdna_dirs():
    """
    Prepare 101 dirs same as UCF-101 dir structure.
    """
    video_dirs = glob.glob(os.path.join(ROOT_DIR, 'UCF-101', '*'))
    if not os.path.exists(VIDEO_FEATURE_DIR):
        os.mkdir(VIDEO_FEATURE_DIR)
    for each in video_dirs:
        dirname, basename = os.path.split(each)
        if os.path.exists( os.path.join(VIDEO_FEATURE_DIR, basename) ):
            continue
        else:
            os.makedirs( os.path.join(VIDEO_FEATURE_DIR, basename) )

def extract_video_features(video_files):
    """
    video_files: a file contains the video path: /UCF-101/Video1/v_video_g01_01.avi
    This function read the file, get the video, replace the UCF-101 with VIDEO_FEATURE_DIR
    The vdna file has the postprefix with .incep3vdna
    """
    with open(video_files) as f:
        videos = f.readlines()
    pbar = tqdm(total=len(videos))
    for each in videos:
        try:
            video = os.path.join(ROOT_DIR, each.strip())
            if not os.path.exists(video):
                raise IOError('Can not found {}'.format(video)) 
            _, dirname, filename = each.strip().split('/')
            dna_path = os.path.join(VIDEO_FEATURE_DIR, dirname, filename)
            dna_path = os.path.splitext(dna_path)[0] + '.incep3vdna'
            if os.path.exists(dna_path):
                continue
            else:
                sequences, info = generate_video_features(video, model)
                with open(dna_path, 'wb') as dnaf:
                    pickle.dump( [sequences, info], dnaf )
        except IOError as reason:
            print("No found", reason)
        finally:
            pbar.update(1)

    pbar.close()

if __name__ == '__main__':
    if False:
        prepare_vdna_dirs()
        start_time = time.time()
        extract_video_features(VIDEO_FILES)
        print('Totally costs {}s'.format( time.time() - start_time ))
    else:
        video_path = '/Users/li_pengju/SomeDownload/Dataset/UCF101/UCF-101/Hammering/v_Hammering_g01_c01.avi'
        start = time.time()
        sequences, info = generate_video_features(video_path, model)
        dna_path = video_path.split('/')[-1]
        dna_path = os.path.splitext(dna_path)[0] + '.incep3vdna'
        with open(dna_path,'wb') as f:
            pickle.dump([sequences,info], f)
        print("cost {}s".format(time.time() - start))
        print(len(sequences), len(sequences[0]))
