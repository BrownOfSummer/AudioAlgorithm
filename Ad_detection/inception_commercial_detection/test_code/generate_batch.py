from __future__ import print_function
import os
import numpy as np
import random
import pickle
def decode_vdna(pickle_filename):
    """
    Load from pickle
    features = [total_frames, 2048]
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
    features = features_info[0]
    info = features_info[1]
    features = [ list(each) for each in features ]
    return features, info

def split_train_test(filepath):
    with open(filepath, 'r') as f:
        data = f.readlines()
    random.shuffle(data)
    split = int(len(data) * 0.9)
    train = data[:split]
    test = data[split:]
    with open(filepath + '-train', 'w') as f:
        for each in train:
            f.write(each)
    with open(filepath + '-test', 'w') as f:
        for each in test:
            f.write(each)

def batch_iter2(data, batch_size, num_epochs=1, shuffle=True):
    """
    Generates a batch iterator for a dataset.
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
        for batch_num in range(num_batches_per_epoch):
            start_index = batch_num * batch_size
            #end_index = min((batch_num + 1) * batch_size, data_size)
            end_index = (batch_num + 1) * batch_size
            if end_index > data_size: continue
            yield shuffled_data[start_index:end_index]

def batch_iter(data, batch_size, num_epochs=1, fixed=False, shuffle=True):
    """
    Generates a batch iterator for a dataset.
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
    film:[[2048], [2048]...n1]
    commercial: [[2048],[2048],...n2]
    """
    num = min( len(film), len(commercial) )
    num = int( num / sample_size )
    print(len(film),len(commercial),num)
    half = int(sample_size / 2)
    data_film = []
    data_commercial = []
    data_mix = []

    if num < 1 or half < 1:
        #return None, None, None
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
    print('label shape',np.shape(label),'; data shape:', np.shape(data))

    data_label = np.array( zip( data, label ) )
    print('data_label.shape=',np.shape(data_label))
    if shuffle:
        shuffle_indices = np.random.permutation(np.arange( 3 * num ))
        shuffled_data_label = data_label[shuffle_indices]
    else:
        shuffled_data_label = data_label
    
    return shuffled_data_label #ndarray

def test_vdna_to_sample():
    filepath = '../data/small_data/commercial.txt-test'
    with open(filepath, 'r') as f:
        dnas = f.readlines()
    dna_com = dnas[0].strip()
    dna_com, _ = decode_vdna(dna_com)

    filepath = '../data/small_data/film.txt-test'
    with open(filepath, 'r') as f:
        dnas = f.readlines()
    dna_film = dnas[0].strip()
    dna_film, _ = decode_vdna(dna_film)

    print('film: {}'.format(np.array(dna_film).shape))
    print('commercial: {}'.format(np.array(dna_com).shape))
    #data_labels = train_sample_iter(dna_film, dna_com)
    data_labels = vdna_to_sample(dna_film, dna_com)
    print('data_labels.shape={}'.format( np.shape(data_labels) ))
    data, labels = zip(*data_labels)
    print(np.shape(data), np.shape(labels))
    for i in range(len(labels)):
        print('data.shape={}, label.shape={}'.format( np.shape(data[i]), np.shape(labels[i]) ))

def generate_batch():
    filepath = '../data/small_data/commercial.txt-test'
    with open(filepath, 'r') as f:
        dnas = f.readlines()
    dna_com = dnas[0].strip()
    dna_com, _ = decode_vdna(dna_com)

    filepath = '../data/small_data/film.txt-test'
    with open(filepath, 'r') as f:
        dnas = f.readlines()
    dna_film = dnas[0].strip()
    dna_film, _ = decode_vdna(dna_film)

    print('film: {}'.format(np.array(dna_film).shape))
    print('commercial: {}'.format(np.array(dna_com).shape))
    #data_labels = train_sample_iter(dna_film, dna_com)
    data_labels = vdna_to_sample(dna_film, dna_com)
    print('data_labels.shape={}'.format( np.shape(data_labels) ))
    #data, labels = zip(*data_labels)
    #print(np.shape(data), np.shape(labels))
    batches = batch_iter(data_labels, 64)
    for batch in batches:
        data_batch, label_batch = zip(*batch)
        print('data_batch.shape={}; label_batch.shape={}'.format(np.shape(data_batch), np.shape(label_batch)))
def test_decode_vdna():
    filepath = '../data/small_data/film.txt-test'
    with open(filepath, 'r') as f:
        dnas = f.readlines()
    dna_film = dnas[0].strip()
    dna_film, info = decode_vdna(dna_film)
    print('Type: ', type(dna_film), type(info))
    print('Type[0]: ', type(dna_film[0]) )
    print('shape:', np.shape(dna_film), np.shape(dna_film[0]))
    aa = [range(2048), range(2048), range(2048)]
    print('aa',np.shape(aa), type(aa))
    print('aa[0]',np.shape(aa[0]), type(aa[0]))
    bb = sum(aa, [])

    tmp = sum(dna_film, [])
    tmp = reduce( lambda c, x: list(c) + x, dna_film, [] )


if __name__ == '__main__':
    #filepath = '../data/small_data/film.txt'
    #filepath = '../data/small_data/commercial.txt'
    #split_train_test(filepath)
    #test_decode_vdna()
    generate_batch()
