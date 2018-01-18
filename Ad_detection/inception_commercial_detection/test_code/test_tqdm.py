from tqdm import tqdm
import time
import sys

pbar = tqdm(total=100)
for _ in range(100):
    for i in range(50):
        sys.stdout.write('\r')
        percentage = 1.0 * ( i + 1 ) / 50
        progress = int(percentage * 30)
        bar_arg = [progress*'=', ' '*(30-progress), percentage*100]
        sys.stdout.write('[{}>{}]{:.0f}%'.format(*bar_arg))
        sys.stdout.flush()
        time.sleep(0.01)

    pbar.update(1)
pbar.close()
"""
    for i, file in enumerate(annotations):
        #progess bar
        sys.stdout.write('\r')
        percentage = 1.0 * (i + 1) / size
        progress = int(percentage * 20)
        bar_arg = [progress*'=', ' '*(20-progress), percentage*100]
        bar_arg +=[file]
        sys.stdout.write('[{}>{}]{:.0f}%  {}'.format(*bar_arg))
        sys.stdout.flush()
"""
