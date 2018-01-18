import subprocess
import datetime
import numpy as np

import cv2

THREAD_NUM=4

def get_video_info(fileloc) :
    command = ['ffprobe',
               '-v', 'fatal',
               '-show_entries', 'stream=width,height,r_frame_rate,duration, nb_frames',
               '-of', 'default=noprint_wrappers=1:nokey=1',
               fileloc, 
               #'-sexagesimal'
               ]
    ffmpeg = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE )
    out, err = ffmpeg.communicate()
    if(err) : print(err)
    out = out.split('\n')
    return {'file' : fileloc,
            'width': int(out[0]),
            'height' : int(out[1]),
            'fps': float(out[2].split('/')[0])/float(out[2].split('/')[1]),
            'duration' : out[3], 
            'frame_count': out[4]}


def get_video_frame_count(fileloc) : # This function is spearated since it is slow.
    command = ['ffprobe',
               '-v', 'fatal',
               '-count_frames',
               '-show_entries', 'stream=nb_read_frames',
               '-of', 'default=noprint_wrappers=1:nokey=1',
               fileloc, '-sexagesimal']
    ffmpeg = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE )
    out, err = ffmpeg.communicate()
    if(err) : print(err)
    out = out.split('\n')
    return {'file' : fileloc,
            'frames' : out[0]}

def read_frame(fileloc,frame,fps,num_frame,t_w,t_h) :
    command = ['ffmpeg',
               '-loglevel', 'fatal',
               '-ss', str(datetime.timedelta(seconds=frame/fps)),
               '-i', fileloc,
               #'-vf', '"select=gte(n,%d)"'%(frame),
               '-threads', str(THREAD_NUM),
               '-vf', 'scale=%d:%d'%(t_w,t_h),
               '-vframes', str(num_frame),
               '-f', 'image2pipe',
               '-pix_fmt', 'rgb24',
               '-vcodec', 'rawvideo', '-']
    #print(command)
    ffmpeg = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE )
    out, err = ffmpeg.communicate()
    if(err) : print('error',err); return None;
    video = np.fromstring(out, dtype='uint8').reshape((num_frame,t_h,t_w,3)) #NHWC
    return video

def read_frame_by_frame(fileloc):
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

    for _ in range(total_frames):
        # read height*width*3 bytes (= 1 frame)
        raw_image = pipe.stdout.read(height*width*3)
        # transform the byte read into a numpy array
        image = np.fromstring(raw_image, dtype='uint8')
        image = image.reshape((height,width,3))
        # throw away the data in the pipe's buffer.
        pipe.stdout.flush()
        cv2.imshow('image', image)
        cv2.waitKey(30)


if __name__ == '__main__':
    video_path = '/Users/li_pengju/SomeDownload/Dataset/uray-4v/a/10/datadir0/a-10.mp4'
    #video_path = '/Users/li_pengju/SomeDownload/Dataset/UCF101/UCF-101/Hammering/v_Hammering_g01_c01.avi'
    info = get_video_info(video_path)
    print(info)
    #video = read_frame(video_path, 0, float(info['fps']), int(info['frame_count']), int(info['height']), int(info['width']))
    read_frame_by_frame(video_path)
    
