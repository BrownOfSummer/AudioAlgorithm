# -*- coding:utf-8 -*-
import tensorflow as tf
import numpy as np

class AdCNN(object):
    """
    A CNN for Ad detection
    Size for outputs:
        (1) for conv:
        W_out = ceil( float( W - ksize + 1) / strides ); VALID
        H_out = ceil( float( H - ksize + 1) / strides ); VALID
        W_out = ceil( float(W) / strides ); SAME
        H_out = ceil( float(H) / strides ); SAME
        (2) for pooling
        same as conv
    """
    def __init__(self, sequence_length=124, num_classes=2, l2_reg_lambda=0.0):
        self.input_x = tf.placeholder(tf.float32, shape=[None, sequence_length], name='input_x')
        self.input_y = tf.placeholder(tf.float32, shape=[None, num_classes], name='input_y')
        self.dropout_keep_prob = tf.placeholder(tf.float32, name='dropout_keep_prob')

        l2_loss = tf.constant(0.0)

        with tf.name_scope('conv1'):
            conv1_weights = tf.Variable(
                    tf.truncated_normal([1, 3, 1, 128],  # 1x5 filter, input depth 1, output depth 128.
                    stddev=0.1,
                    dtype=tf.float32))
            conv1_biases = tf.Variable(tf.constant(0.1, shape=[128], dtype=tf.float32), name='conv1_biases')
            input_x = tf.reshape(self.input_x, [-1, 1, 124, 1])
            #input_x = tf.expand_dims(input_x, -1) #[batch, 1, 124, 1]
            conv1 = tf.nn.conv2d(input_x, 
                                conv1_weights,
                                strides=[1,1,1,1],
                                padding='SAME')
            relu1 = tf.nn.relu(tf.nn.bias_add(conv1, conv1_biases)) # [batch, 1, 124, 128]

        with tf.name_scope('conv2'):
            conv2_weights = tf.Variable(
                    tf.truncated_normal([1, 3, 128, 256],
                    stddev=0.1,
                    dtype=tf.float32))
            conv2_biases = tf.Variable(tf.constant(0.1, shape=[256], dtype=tf.float32), name='conv2_biases')
            conv2 = tf.nn.conv2d(relu1, 
                                 conv2_weights,
                                 strides=[1,1,1,1],
                                 padding='SAME')
            relu2 = tf.nn.relu(tf.nn.bias_add(conv2, conv2_biases)) #[batch, 1, 124, 256]

        # outsize_w = (w - ksize) / strides + 1
        # outsize_h = (h - ksize) / strides + 1
        with tf.name_scope('pool1'):
            pool1 = tf.nn.max_pool(
                                    relu2,
                                    ksize=[1, 1, 2, 1],
                                    strides=[1,1,1,1],
                                    padding='VALID',
                                    name='pool1') #[batch, 1, 123, 256]===> (124 - 2 + 1)/1=123
        with tf.name_scope('conv3'):
            conv3_weights = tf.Variable(
                    tf.truncated_normal([1, 3, 256, 512], stddev=0.1, seed=None, dtype=tf.float32))
            conv3_biases = tf.Variable(tf.constant(0.1, shape=[512], dtype=tf.float32), name='conv3_biases')
            conv3 = tf.nn.conv2d(pool1, conv3_weights, strides=[1,1,1,1], padding='SAME')
            relu3 = tf.nn.relu(tf.nn.bias_add(conv3, conv3_biases)) #[batch, 1, 123, 512]
        
        with tf.name_scope('pool2'):
            pool2 = tf.nn.max_pool(
                                    relu3,
                                    ksize=[1, 1, 2, 1],
                                    strides=[1, 1, 2, 1],
                                    padding='VALID',
                                    name='pool2') #[batch, 1, 61, 512]
        with tf.name_scope('conv4'):
            conv4_weights = tf.Variable( tf.truncated_normal([1, 3, 512, 1024], stddev=0.1, seed=None, dtype=tf.float32) )
            conv4_biases = tf.Variable( tf.constant(0.1, shape=[1024], dtype=tf.float32), name='conv4_biases' )
            conv4 = tf.nn.conv2d(pool2, conv4_weights, strides=[1,1,1,1], padding='SAME')
            relu4 = tf.nn.relu( tf.nn.bias_add(conv4, conv4_biases) ) #[batch, 1, 61, 1024]
        
        with tf.name_scope('pool3'):
            pool3 = tf.nn.max_pool(
                                    relu4,
                                    ksize=[1, 1, 2, 1],
                                    strides=[1, 1, 2, 1],
                                    padding='VALID',
                                    name='pool3') #[batch, 1, 30, 1024]
        
        pool_flat = tf.reshape(pool3, [-1, 30*1024])

        with tf.name_scope('dropout1'):
            drop = tf.nn.dropout(pool_flat, self.dropout_keep_prob)

        with tf.name_scope('pool4'):
            drop = tf.reshape( drop, shape=[-1, 1, 30, 1024] )
            pool4 = tf.nn.max_pool( drop, ksize=[1, 1, 30, 1], strides=[1,1,1,1], padding='VALID', name='pool4' ) #[batch, 1, 1, 1024]
        
        #****************************************#
        # pool_shape = pool.get_shape().as_list()
        # pool_flat = tf.reshape(pool, [pool_shape[0], pool_shape[1] * pool_shape[2] * pool_shape[3]])
        #****************************************#

        pool_flat = tf.reshape(pool4, shape=[-1, 1024])
        with tf.name_scope('dropout2'):
            self.drop = tf.nn.dropout(pool_flat, self.dropout_keep_prob, name='dropout2')


        #pool_shape = pool.get_shape().as_list()
            

        with tf.name_scope('output'):
            W = tf.Variable( tf.truncated_normal([1024, num_classes], stddev=0.1, dtype=tf.float32), name='W')
            b = tf.Variable(tf.constant(0.1, shape=[num_classes]), name='b')
            l2_loss += tf.nn.l2_loss(W)
            l2_loss += tf.nn.l2_loss(b)
            self.scores = tf.nn.xw_plus_b(self.drop, W, b, name='scores')
            self.predictions = tf.argmax(self.scores, 1, name='predictions')
            self.probability = tf.nn.softmax(self.scores, name='probability')
            
        with tf.name_scope('loss'):
            #cross_entropy = tf.nn.softmax_cross_entropy_with_logits(logits=self.scores,labels=self.input_y)
            cross_entropy = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=self.scores,labels=self.input_y))
            #cross_entropy = tf.reduce_mean(-tf.reduce_sum(self.input_y * tf.log(self.probability), reduction_indices=[1]))
            self.loss = cross_entropy
            #self.loss = tf.add(cross_entropy, l2_reg_lambda * l2_loss, name='loss')
        

        self.train_op = tf.train.AdamOptimizer(0.001).minimize(self.loss)
        
        with tf.name_scope('accuracy'):
            correct_predictions = tf.equal(self.predictions, tf.argmax(self.input_y, 1))
            self.accuracy = tf.reduce_mean(tf.cast(correct_predictions, "float"), name="accuracy")
