from __future__ import print_function
import os
import pickle
import glob
import numpy as np
def split_train_test(filepath, ratio=0.8):
    """
    split raw file to train and test file with ratio
    """
    with open(filepath) as f:
        data = f.readlines()
    data = np.array(data)
    data_size = len(data)
    shuffle_indices = np.random.permutation(np.arange(data_size))
    data = data[shuffle_indices]
    trainfile = filepath.strip() + '-train'
    testfile = filepath.strip() + '-test'
    if ratio > 0 and ratio < 1:
        train_data, test_data = list(data[:int(data_size * ratio)]), list(data[int(data_size * ratio):])
        with open(trainfile, 'w') as f1:
            f1.writelines(train_data)
        with open(testfile, 'w') as f2:
            f2.writelines(test_data)
    else:
        raise ValueError('ratio must not more than 1.0 and no less than 0.0')

def convert2dataset(filepath):
    """
    Read a file and convert to dataset with both data and labels.
    """
    data = []
    labels = []
    with open(filepath, 'r') as f:
        for each in f:
            each = each.strip().split()
            label, line = each[0], each[1:]
            if label == '1':
                labels.append([0, 1])
            elif label == '-1':
                labels.append([1, 0])
            else:
                raise ValueError('class should be 1 or -1')
            
            tmp = [ 0 for _ in range(124) ]
            for item in line:
                key, value = item.split(':')
                key, value = int(key), float(value)
                if key < 123:
                    tmp[key - 1] = value
                elif key == 4124:
                    tmp[122] = value
                elif key == 4125:
                    tmp[123] = value
                else:
                    pass
            data.append(tmp)
    return data, labels

def save_data_label(data_label=None, name=None):
    """
    Pickle the processed data and label.
    data_label = [data, labels]
    """
    if not data_label or not name:
        print("Nothing to be save !")
        return
    else:
        with open(name, 'wb') as f:
            pickle.dump(data_label, f)

def load_data_label(pickle_filename):
    """
    Load from pickle
    """
    with open(pickle_filename, 'rb') as f:
        data_label = pickle.load(f)
    return data_label


def batch_iter(data, batch_size, num_epochs, shuffle=True):
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
            end_index = min((batch_num + 1) * batch_size, data_size)
            yield shuffled_data[start_index:end_index]
