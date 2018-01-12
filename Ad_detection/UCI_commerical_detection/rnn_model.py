# -*- coding:utf-8 -*-
import tensorflow as tf
import numpy as np


class AdLSTM(object):
    def __init__(self, timestep_size=4, input_size=31, hidden_size=64, num_classes=2, l2_reg_lambda=0.0):
        self.hidden_size = hidden_size
        self.timestep_size = timestep_size
        self.input_size = input_size
        self.num_classes = num_classes
        self.l2_reg_lambda = l2_reg_lambda
        with tf.name_scope('inputs'):
            self.input_x = tf.placeholder(tf.float32, shape=[None, timestep_size*input_size], name='input_x')
            self.input_y = tf.placeholder(tf.float32, shape=[None, num_classes], name='input_y')
            
            self.X = tf.reshape(self.input_x, [-1, timestep_size, input_size], name='lstm_input')
            
            # use different batch_size when train and valid or test
            self.batch_size = tf.placeholder(tf.int32, [])
        self.dropout_keep_prob = tf.placeholder(tf.float32, name='dropout_keep_prob')

        l2_loss = tf.constant(0.0)

        with tf.name_scope('LSTM'):
            mlstm_cell = tf.nn.rnn_cell.MultiRNNCell([self.unit_lstm() for i in range(3)],
                    state_is_tuple=True)
            #用全零来初始化state
            init_state = mlstm_cell.zero_state(self.batch_size, dtype=tf.float32)
            outputs, state = tf.nn.dynamic_rnn(mlstm_cell, inputs=self.X, initial_state=init_state, time_major=False)

            """
            当 time_major==False 时， outputs.shape = [batch_size, timestep_size, hidden_size]
            所以，可以取 h_state = outputs[:, -1, :] 作为最后输出
            state.shape = [layer_num, 2, batch_size, hidden_size],
            或者，可以取 h_state = state[-1][1] 作为最后输出
            最后输出维度是 [batch_size, hidden_size]
            """
            h_state = outputs[:, -1, :]  # 或者 h_state = state[-1][1]

        with tf.name_scope('output'):
            W = tf.Variable( tf.truncated_normal([self.hidden_size, self.num_classes], stddev=0.1, dtype=tf.float32), name='W')
            b = tf.Variable(tf.constant(0.1, shape=[self.num_classes]), name='b')
            l2_loss += tf.nn.l2_loss(W)
            l2_loss += tf.nn.l2_loss(b)
            self.scores = tf.nn.xw_plus_b(h_state, W, b, name='scores')
            self.predictions = tf.argmax(self.scores, 1, name='predictions')
            self.probability = tf.nn.softmax(self.scores, name='probability')

        with tf.name_scope('loss'):
            cross_entropy = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=self.scores,labels=self.input_y))
            #cross_entropy = -tf.reduce_mean(self.input_y * tf.log(self.probability))
            #self.loss = cross_entropy
            self.loss = tf.add(cross_entropy, self.l2_reg_lambda * l2_loss, name='loss')

        self.train_op = tf.train.AdamOptimizer(0.001).minimize(self.loss)

        with tf.name_scope('accuracy'):
            correct_predictions = tf.equal(self.predictions, tf.argmax(self.input_y, 1))
            self.accuracy = tf.reduce_mean(tf.cast(correct_predictions, "float"), name="accuracy")
    
    
    def unit_lstm(self):
        # 定义一层 LSTM_cell，只需要说明 hidden_size, 它会自动匹配输入的 X 的维度
        lstm_cell = tf.nn.rnn_cell.BasicLSTMCell(num_units=self.hidden_size, 
                forget_bias=1.0, 
                state_is_tuple=True)

        # 添加 dropout layer, 一般只设置 output_keep_prob
        lstm_cell = tf.nn.rnn_cell.DropoutWrapper(cell=lstm_cell, 
                input_keep_prob=1.0, output_keep_prob=self.dropout_keep_prob)

        return lstm_cell


