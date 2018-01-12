# -*- coding:utf-8 -*-
from __future__ import print_function
import os
import numpy as np 
import time
from datetime import datetime
import tensorflow as tf
from data_helpers import load_data_label, batch_iter
from rnn_model import AdLSTM

TRAIN_BATCH = 256
VALID_BATCH = 1000
TEST_BATCH = 1

def cnn_train(train_val_data_path):
    print('Loading and decode data...')
    trainval = load_data_label(train_val_data_path)
    x_trainval, y_trainval = np.array(trainval[0]), np.array(trainval[1])
    data_size = len(y_trainval)
    train_len = int(data_size - VALID_BATCH)
    shuffle_indices = np.random.permutation( np.arange(data_size) )
    x_trainval, y_trainval = x_trainval[shuffle_indices], y_trainval[shuffle_indices]
    x_train, y_train = x_trainval[:train_len], y_trainval[:train_len]
    x_valid, y_valid = x_trainval[train_len:], y_trainval[train_len:]
    print('Load {} training data; {} labels.'.format(len(x_train), len(y_train)))
    print('Load {} validing data; {} labels.'.format(len(x_valid), len(y_valid)))

    print('Start training')
    start_train = time.time()

    with tf.Graph().as_default():
        session_conf = tf.ConfigProto(
                allow_soft_placement=True,
                log_device_placement=False)
        sess = tf.Session(config=session_conf)

        with sess.as_default():
            #cnn = AdCNN(sequence_length=124, num_classes=2, l2_reg_lambda=0.0)
            cnn = AdLSTM(timestep_size=4, input_size=31, hidden_size=256, num_classes=2, l2_reg_lambda=0.0)

            # Define training procedure
            global_step = tf.Variable(0, name="global_step", trainable=False)
            optimizer = tf.train.AdamOptimizer(1e-3)
            grads_and_vars = optimizer.compute_gradients(cnn.loss)
            train_op = optimizer.apply_gradients(grads_and_vars, global_step=global_step)
            tf.add_to_collection('train_op', train_op)

            # Output directory for models and summaries
            out_dir = os.path.abspath('./training')
            #if os.path.exists(out_dir):
            #    gfile.DeleteRecursively(out_dir)
            print("Writing to {}\n".format(out_dir))
            
            # Summaries for loss and accuracy
            loss_summary = tf.summary.scalar('loss', cnn.loss)
            acc_summary = tf.summary.scalar('accuracy', cnn.accuracy)

            valid_acc = tf.summary.scalar('valid_acc', cnn.accuracy)

            # Train Summaries
            train_summary_op = tf.summary.merge([loss_summary, acc_summary])
            train_summary_dir = os.path.join(out_dir, "summaries", "train")
            train_summary_writer = tf.summary.FileWriter(train_summary_dir, sess.graph)

            # Checkpoint directory. Tensorflow assumes this directory already exists so we need to create it
            checkpoint_dir = os.path.abspath(os.path.join(out_dir, "checkpoints"))
            checkpoint_prefix = os.path.join(checkpoint_dir, "model")
            if not os.path.exists(checkpoint_dir):
                os.makedirs(checkpoint_dir)
            saver = tf.train.Saver(tf.global_variables(), max_to_keep=2)
            
            sess.run(tf.global_variables_initializer())
            
            def train_step(x_batch, y_batch):
                """
                A single training step
                """
                feed_dict = {
                  cnn.input_x: x_batch,
                  cnn.input_y: y_batch,
                  cnn.dropout_keep_prob: 0.8,
                  cnn.batch_size: TRAIN_BATCH
                }
                _, step, summaries, loss, accuracy = sess.run(
                    [train_op, global_step, train_summary_op, cnn.loss, cnn.accuracy],
                    feed_dict=feed_dict)
                time_str = datetime.now().isoformat()
                print("{}: step {}, loss {}, train_acc {}".format(time_str, step, loss, accuracy))
                train_summary_writer.add_summary(summaries, step)

            def valid_step(x_batch, y_batch):
                feed_dict = {
                        cnn.input_x: x_batch,
                        cnn.input_y: y_batch,
                        cnn.dropout_keep_prob: 1,
                        cnn.batch_size: VALID_BATCH}
                accuracy, summaries, step = sess.run([cnn.accuracy, valid_acc, global_step], feed_dict=feed_dict)
                train_summary_writer.add_summary(summaries, step)
                time_str = datetime.now().isoformat()
                print("{}: step {}, valid_acc {}".format(time_str, step, accuracy))
            
            # Generate batches
            batches = batch_iter(
                    list(zip(x_train, y_train)), TRAIN_BATCH, 20) #batchsize=64, 20 epoch
            # Training loop. For each batch...

            for batch in batches:
                x_batch, y_batch = zip(*batch)
                train_step(x_batch, y_batch)
                current_step = tf.train.global_step(sess, global_step)
                if current_step % 100 == 0:
                    valid_step(x_valid, y_valid)
                    path = saver.save(sess, checkpoint_prefix, global_step=current_step)
                    print("Saved model checkpoint to {}\n".format(path))
    
    end_train = time.time()
    print('Train costs', end_train - start_train)


if __name__ == '__main__':
    #trainval_data_path='../data/trainval.pickle'
    trainval_data_path='./data/test.pickle'
    cnn_train(trainval_data_path)
