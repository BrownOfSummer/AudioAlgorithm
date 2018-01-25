from helpers import decode_vdna

if __name__ == '__main__':
    vdna_file = './v_Hammering_g01_c01.incep3vdna'
    features, info = decode_vdna(vdna_file)
    width, height = int(info['width']), int(info['height'])
    duration = float(info['duration'])
    total_frames = int(info['frame_count'])
    fps = float(info['fps'])
    print("Video length: {}s;\nVideo Resolution: {} x {};".format(duration, height, width))
    print("Video frames: {};\nVideo fps: {};".format(total_frames, fps))
    print(info['file'])
    video_class = info['file'].split('/')[-2]
    video_name = info['file'].split('/')[-1]
    print('Video class: {}, video name: {}'.format( video_class, video_name ))
    print(len(features), len(features[0]))
