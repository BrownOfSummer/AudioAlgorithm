from __future__ import print_function
import os

if __name__ == '__main__':
    dirpath = '/Users/li_pengju/Downloads/TV_News_Channel_Commercial_Detection_Dataset'
    txtfile = 'TIMESNOW.txt'
    file_path = os.path.join(dirpath, txtfile)
    count = 0
    with open(file_path, 'r') as fb:
        for each in fb:
            each = each.strip().split()     
            elems = [i for i in each]
            shot_length = int(elems[0])
            if shot_length != 1:
                #print(elems)
                print(shot_length, len(elems))
            #if count == 10:
            #    break
            #count += 1
