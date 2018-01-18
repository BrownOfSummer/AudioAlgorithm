# -*- coding:utf-8 -*-
from __future__ import print_function
import tensorflow as tf
import numpy as np
import time
import os

class AdRNN(object):
    def __init__(self, time_step=4, seq_length=2048, num_classes=2,
            lstm_size=128, num_layers=3, learning_rate=0.001,
            grad_clip=5):
        
        self.num_classes = num_classes
        self.time_step = time_step
        self.seq_length = seq_length
        self.lstm_size = lstm_size
        self.num_layers = num_layers
        self.learning_rate = learning_rate
        self.grad_clip = grad_clip
        #self.train_keep_prob = train_keep_prob

        #tf.reset_default_graph()
        self.build_inputs()
        self.build_lstm()
        self.build_outputs()
        self.build_loss()
        self.build_optimizer()

    def build_inputs(self):
        with tf.name_scope('inputs'):
            self.input_x = tf.placeholder(tf.float32, shape=[None, self.time_step * self.seq_length], name='input_x')
            self.input_y = tf.placeholder(tf.float32, shape=[None, self.num_classes], name='input_y')
            
            self.X = tf.reshape(self.input_x, [-1, self.time_step, self.seq_length], name='lstm_input')
            
            # use different batch_size when train and valid or test
            self.batch_size = tf.placeholder(tf.int32, [])
            self.dropout_keep_prob = tf.placeholder(tf.float32, name='dropout_keep_prob')

    def build_lstm(self):
        # create a single lstm cell
        def get_unit_cell(lstm_size, keep_prob):
            lstm_cell = tf.nn.rnn_cell.BasicLSTMCell(num_units=lstm_size,
                                                forget_bias=1.0,
                                                state_is_tuple=True)
            lstm_cell = tf.nn.rnn_cell.DropoutWrapper(cell=lstm_cell,
                                                    input_keep_prob=1.0,
                                                    output_keep_prob=keep_prob)
            return lstm_cell

        with tf.name_scope('LSTM'):
            mlstm_cell = tf.nn.rnn_cell.MultiRNNCell(
                    [get_unit_cell(self.lstm_size, self.dropout_keep_prob) for _ in range(self.num_layers)])
            self.initial_state = mlstm_cell.zero_state(self.batch_size, dtype=tf.float32)
            self.lstm_outputs, self.final_state = tf.nn.dynamic_rnn(mlstm_cell, self.X, initial_state=self.initial_state)

            #seq_output = tf.concat(self.lstm_outputs, 1)
            #x = tf.reshape(seq_output, [-1, self.lstm_size])

    def build_outputs(self):
        with tf.name_scope('output'):
            with tf.variable_scope('softmax'):
                softmax_w = tf.Variable(tf.truncated_normal([self.lstm_size, self.num_classes], stddev=0.1))
                softmax_b = tf.Variable(tf.constant(0.1, shape=[self.num_classes]), name='b')
            with tf.name_scope('reshape_rnn_out'):
                #seq_output = tf.concat(self.lstm_outputs, 1)
                #x = tf.reshape(seq_output, [-1, self.lstm_size])
                h_state = self.lstm_outputs[:, -1, :]  # Or h_state = state[-1][1]
            self.logits = tf.nn.xw_plus_b( h_state, softmax_w, softmax_b, name='logits')
            self.predictions = tf.argmax(self.logits, 1, name='predictions')
            self.probability = tf.nn.softmax(self.logits, name='probability')

            correct_predictions = tf.equal(self.predictions, tf.argmax(self.input_y, 1))
            self.accuracy = tf.reduce_mean(tf.cast(correct_predictions, "float"), name="accuracy")
    
    def build_loss(self):
        with tf.name_scope('loss'):
            cross_entropy = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(
                logits=self.logits,labels=self.input_y))
            self.loss = cross_entropy

    def build_optimizer(self):
        # using clipping gradients
        with tf.name_scope('optimizer'):
            self.global_step = tf.Variable(0, name="global_step", trainable=False)
            tvars = tf.trainable_variables()
            grads, _ = tf.clip_by_global_norm(tf.gradients(self.loss, tvars), self.grad_clip)
            train_op = tf.train.AdamOptimizer(self.learning_rate)
            self.optimizer = train_op.apply_gradients(zip(grads, tvars), global_step=self.global_step)
