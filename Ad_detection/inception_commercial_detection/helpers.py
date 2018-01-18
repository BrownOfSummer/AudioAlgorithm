# -*- coding:utf-8 -*-
import subprocess
import numpy as np

def get_video_info(fileloc):
    """
    Input a video and return some video information as a dict
    """
    command = ['ffprobe',
               '-v', 'fatal',
               '-show_entries', 'stream=width,height,r_frame_rate,duration, nb_frames',
               '-of', 'default=noprint_wrappers=1:nokey=1',
               fileloc, 
               #'-sexagesimal'
               ]
    ffmpeg = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE )
    out, err = ffmpeg.communicate()
    if(err): 
        print(err) 
        return None
    out = out.split('\n')
    return {'file' : fileloc,
            'width': int(out[0]),
            'height' : int(out[1]),
            'fps': float(out[2].split('/')[0])/float(out[2].split('/')[1]),
            'duration' : out[3], 
            'frame_count': out[4]}

def generate_video_features(fileloc, model):
    """
    Input a video and a model, to extract the features frame by frame.
    Return the features and video information.
    """
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
