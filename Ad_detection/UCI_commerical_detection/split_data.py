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
        raise ValueError

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
                raise ValueError
            
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


def _test_split():
    rootpath = '/Users/li_pengju/Downloads/TV_News_Channel_Commercial_Detection_Dataset/'
    files = ['BBC.txt', 'CNN.txt', 'CNNIBN.txt', 'NDTV.txt', 'TIMESNOW.txt']
    for each in files:
        filepath = os.path.join(rootpath, each)
        split_train_test(filepath, 0.9)
def _test_save_load():
    train_val_files = glob.glob('./data/*-train')
    #train_val_files = ['./text_data.txt']
    test_files = glob.glob('./data/*-test')
    trainVal = []
    trainVal_labels = []
    test = []
    test_labels = []
    for each in train_val_files:
        print(each)
        data, labels = convert2dataset(each)
        trainVal += data
        trainVal_labels += labels
    
    save_data_label([trainVal, trainVal_labels], './data/trainval.pickle')
    print("Totally get ",len(trainVal), " train elements;","\t",len(trainVal_labels), "labels")
    tmp = load_data_label('./data/trainval.pickle')
    trainVal, trainVal_labels = tmp[0], tmp[1]
    print("Totally load ",len(trainVal), " train elements;","\t",len(trainVal_labels), "labels")

    for each in test_files:
        print(each)
        data, labels = convert2dataset(each)
        test += data
        test_labels += labels

    save_data_label([test, test_labels], './data/test.pickle')
    print("Totally get ",len(test), " test elements;","\t",len(test_labels), "labels")
    tmp = load_data_label('./data/test.pickle')
    test, test_labels = tmp[0], tmp[1]
    print("Totally load ",len(test), " test elements;","\t",len(test_labels), "labels")


if __name__ == '__main__':
    #_test_split()
    _test_save_load()
