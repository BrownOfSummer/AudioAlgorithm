"""
load a frozen model and eval the model, if do not draw boxex, 
    from utils import label_map_util
    from utils import visualization_utils as vis_util
can be delete
"""
from __future__ import print_function
import numpy as np
import os
import sys
import time
from PIL import Image
import tensorflow as tf

sys.path.append("..")
from utils import label_map_util
from utils import visualization_utils as vis_util

MODEL_DIR = "~/SomeDownload/models/faster_rcnn_inception_v2_coco_2017_11_08/"
MODEL_DIR = "../faster_inception_v2_7769/"
PATH_TO_CKPT = os.path.join(MODEL_DIR, "frozen_inference_graph.pb")
LABEL_DIR = "../faster_inception_v2_7769/"
PATH_TO_LABELS = os.path.join(LABEL_DIR, "label_map.pbtxt")

NUM_CLASSES = 52
print("Use model in : ", PATH_TO_CKPT)
print("Use label in : ", PATH_TO_LABELS)

"""
Loading label map
"""
print("Loading label map in : ", PATH_TO_LABELS)
label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)

PATH_TO_TEST_IMAGES_DIR = '../TextDataset/JPEGImages/'
TEST_IMAGE_PATHS = [ os.path.join(PATH_TO_TEST_IMAGES_DIR, '000{}.jpg'.format(i)) for i in range(0, 10) ]

# Size, in inches, of the output images.
#IMAGE_SIZE = (12, 8)

def load_image_into_numpy_array(image):
    (im_width, im_height) = image.size
    return np.array(image.getdata()).reshape((im_height, im_width, 3)).astype(np.uint8)

print("Loading model.....")
load_start=time.time()
"""Load a (frozen) Tensorflow model into memory."""
def load_frozen_graph(path_to_frozen_pb):
    detection_graph = tf.Graph()
    with detection_graph.as_default():
        od_graph_def = tf.GraphDef()
        with tf.gfile.GFile(path_to_frozen_pb, 'rb') as fid:
            serialized_graph = fid.read()
            od_graph_def.ParseFromString(serialized_graph)
            tf.import_graph_def(od_graph_def, name='')
    return detection_graph

detection_graph = load_frozen_graph(PATH_TO_CKPT)
load_end = time.time()
print("Load model cost {}.s".format( load_end - load_start ))

with detection_graph.as_default():
    with tf.Session(graph=detection_graph) as sess:
        # Definite input and output Tensors for detection_graph
        image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')
        # Each box represents a part of the image where a particular object was detected.
        detection_boxes = detection_graph.get_tensor_by_name('detection_boxes:0')
        # Each score represent how level of confidence for each of the objects.
        # Score is shown on the result image, together with the class label.
        detection_scores = detection_graph.get_tensor_by_name('detection_scores:0')
        detection_classes = detection_graph.get_tensor_by_name('detection_classes:0')
        num_detections = detection_graph.get_tensor_by_name('num_detections:0')

        for image_path in TEST_IMAGE_PATHS:
            image = Image.open(image_path)
            # the array based representation of the image will be used later in order to prepare the
            # result image with boxes and labels on it.
            image_np = load_image_into_numpy_array(image)
            # Expand dimensions since the model expects images to have shape: [1, None, None, 3]
            image_np_expanded = np.expand_dims(image_np, axis=0)
            # Actual detection.
            (boxes, scores, classes, num) = sess.run(
                [detection_boxes, detection_scores, detection_classes, num_detections],
                feed_dict={image_tensor: image_np_expanded})
            # Visualization of the results of a detection.
            """
            vis_util.visualize_boxes_and_labels_on_image_array(
                image_np,
                np.squeeze(boxes),
                np.squeeze(classes).astype(np.int32),
                np.squeeze(scores),
                category_index,
                use_normalized_coordinates=True,
                line_thickness=5,
                min_score_thresh=0.5,
                max_boxes_to_draw=100)
            #print("boxes = ", np.squeeze(boxes))
            #print("classes = ", np.squeeze(classes).astype(np.int32))
            #print("scores = ", np.squeeze(scores))
            #print(scores)
            img_show = Image.fromarray(image_np)
            img_show.show()
            """
        eval_end = time.time()
        print("Totally {} images, detection and recognition cost {}.s".format(len(TEST_IMAGE_PATHS), eval_end - load_end))
