# -*- coding:utf-8 -*-
import subprocess
import numpy as np
import pickle

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
    ffprobe = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE )
    out, err = ffprobe.communicate()
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
    #print("Video length: {}s;\nVideo Resolution: {} x {};".format(duration, height, width))
    #print("Video frames: {};\nVideo fps: {};".format(total_frames, fps))

    command = ['ffmpeg',
                '-i', fileloc, 
                '-f', 'image2pipe', 
                '-pix_fmt', 'rgb24', 
                '-vcodec', 'rawvideo', '-']
    #pipe = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE, bufsize=10**8 )
    pipe = subprocess.Popen(command, stderr=subprocess.PIPE ,stdout = subprocess.PIPE, bufsize=10**8, close_fds=True )

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

def decode_vdna(pickle_filename):
    """
    Load from pickle
    features = [total_frames, 2048], the data type is [ ndarray for each feature vector ]
    info=
        {
            'width': width,
            'height': height,
            'duration': video_secs,
            'frame_count': total_frames,
            'fps': video fps,
            'file': video_file_path when generate this
        }
    """
    with open(pickle_filename, 'rb') as f:
        features_info = pickle.load(f)
    features, info = features_info[0], features_info[1]
    features = [ list(each) for each in features ]
    return features, info

def batch_iter(data, batch_size, num_epochs=1, fixed=False, shuffle=True):
    """
    Generates a batch iterator for a dataset.
    paras:
        data: input data and label zips.
        batch_size: num elements per batch.
        num_epochs: times to repeat this data.
        fixed: crop end to fixed to batch_size, or return less elements at the end.
        shuffle: shuffle the data.
    """
    data = np.array(data)
    data_size = len(data)
    num_batches_per_epoch = int((len(data)-1)/batch_size) + 1
    for epoch in range(num_epochs):
        # Shuffle the data at each epoch
        if shuffle:
            shuffle_indices = np.random.permutation(np.arange(data_size))
            shuffled_data = data[shuffle_indices]
        else:
            shuffled_data = data

        if fixed:
            for batch_num in range(num_batches_per_epoch):
                start_index = batch_num * batch_size
                end_index = (batch_num + 1) * batch_size
                if end_index > data_size: continue
                yield shuffled_data[start_index:end_index]
        else:
            for batch_num in range(num_batches_per_epoch):
                start_index = batch_num * batch_size
                end_index = min((batch_num + 1) * batch_size, data_size)
                yield shuffled_data[start_index:end_index]

def vdna_to_sample(film, commercial, sample_size=4, shuffle=False):
    """
    Input film and commercial vdnas, 
    slice with sample_size length, generate mixed vdna vector, assign one-hot label.
    
    paras:
        film: film vdnas decoded by decode_vdna func, list: [[2048], [2048]...n1]
        commercial: vdnas decoded by decode_vdna func, list: [[2048],[2048],...n2]
        sample_size: how many frames together as a feature vector.
        shuffle: shuffle the vdna segments or not, can be True when train, False when valid and test.
    """
    num = min( len(film), len(commercial) )
    num = int( num / sample_size )
    half = int(sample_size / 2)
    data_film = []
    data_commercial = []
    data_mix = []

    if num < 1 or half < 1:
        return None
    else:
        for i in range(num):
            """
            Slice the dna with sample_size.
            """
            film_seg = film[sample_size * i : sample_size * (i + 1)] #[[2048], [2048], ... sample_size]
            commercial_seg = commercial[sample_size * i : sample_size * (i + 1)] #[[2048], [2048], ... sample_size]
            if i % 2 == 0:
                mix_seg = film_seg[:half] + commercial_seg[half:] #[[2048], [2048], ... sample_size]
            else:
                mix_seg = commercial_seg[:half] + film_seg[half:]

            """
            Concate the list file.
            """
            film_seg = sum(film_seg, []) #[sample_size * 2048]
            #film_seg = reduce( lambda c,x: x+c, film_seg, [] )
            data_film.append(film_seg)
            #print('film:',np.shape(data_film))

            commercial_seg = sum(commercial_seg, []) #[ sample_size * 2048 ]
            data_commercial.append(commercial_seg)
            #print('comm:',np.shape(data_commercial))

            mix_seg = sum(mix_seg, [])
            data_mix.append( mix_seg )
            #print('mix :', np.shape(data_mix))
    """
    Generate one-hot label.
    """
    label_film = [ [1, 0, 0] for _ in range(num) ]
    label_mix = [ [0, 1, 0] for _ in range(num) ]
    label_commercial = [ [0, 0, 1] for _ in range(num) ]
    
    label = label_film + label_mix + label_commercial #shape=[[2048 * sample_size], 3*num]
    data = data_film + data_mix + data_commercial #shape=[[3], 3*num]
    #print('label shape',np.shape(label),'; data shape:', np.shape(data))

    data_label = np.array( zip( data, label ) )
    #print('data_label.shape=',np.shape(data_label))
    if shuffle:
        shuffle_indices = np.random.permutation(np.arange( 3 * num ))
        shuffled_data_label = data_label[shuffle_indices]
    else:
        shuffled_data_label = data_label
    
    return shuffled_data_label #ndarray
