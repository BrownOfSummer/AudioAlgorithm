# -*- coding:utf-8 -*-
from __future__ import print_function
import os
import numpy as np
import time
import random
import tensorflow as tf

from datetime import datetime
from helpers import decode_vdna, vdna_to_sample, batch_iter
from ad_model import AdRNN

# train config
TIME_STEP=4
TRAIN_BATCH=32
EPOCH = 20

def train_model(film_vdna_files, commercial_vdna_files):

    with tf.Graph().as_default():
        session_conf = tf.ConfigProto(
                allow_soft_placement=True,
                log_device_placement=False)
        sess = tf.Session(config=session_conf)

        with sess.as_default():
            model = AdRNN(
                    time_step=TIME_STEP,
                    seq_length=2048,
                    num_classes=3,
                    num_layers=3,
                    lstm_size=128,
                    learning_rate=0.001)
            global_step = model.global_step
            train_op = model.optimizer

            # Output directory for models and summaries
            out_dir = os.path.abspath('./trained_model')
            print("Writing to {}\n".format(out_dir))
    
            # Summaries for loss and accuracy
            loss_summary = tf.summary.scalar('loss', model.loss)
            acc_summary = tf.summary.scalar('train_acc', model.accuracy)
            valid_acc = tf.summary.scalar('valid_acc', model.accuracy)

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
            
    
            def train_step(feed_dict):
                """
                A single training step
                """
                _, step, summaries, loss, accuracy = sess.run(
                    [train_op, global_step, train_summary_op, model.loss, model.accuracy],
                    feed_dict=feed_dict)
                time_str = datetime.now().isoformat()
                print("{}: step {}, loss {}, train_acc {}".format(time_str, step, loss, accuracy))
                train_summary_writer.add_summary(summaries, step)

            def valid_step(feed_dict):
                accuracy, summaries, step = sess.run([cnn.accuracy, valid_acc, global_step], feed_dict=feed_dict)
                train_summary_writer.add_summary(summaries, step)
                time_str = datetime.now().isoformat()
                print("{}: step {}, valid_acc {}".format(time_str, step, accuracy))
    
            # Generate batches
            with open(film_vdna_files, 'r') as f:
                films = f.readlines()
            with open(commercial_vdna_files, 'r') as f:
                coms = f.readlines()
            lens = min( len(films), len(coms) )
            for _ in range(EPOCH):
                random.shuffle(films)
                random.shuffle(coms)

                train_film_paths = films[:lens]
                train_com_paths = coms[:lens]

                for i in range(lens):
                    film_vdna_path = train_film_paths[i].strip()
                    com_vdna_path = train_com_paths[i].strip()
                    try:
                        film_vdna, _ = decode_vdna( film_vdna_path )
                        com_vdna, _ = decode_vdna( com_vdna_path )
                    except EOFError as reason:
                        print('WARNING, Can not load vdna from:\n\t{}\n\t{}'.format( film_vdna_path, com_vdna_path ))
                        continue

                    combined_samples = vdna_to_sample( film_vdna, com_vdna )
                    batches = batch_iter( combined_samples, TRAIN_BATCH )

                    print("training {} --and-- {}".format( film_vdna_path.split('/')[-1], com_vdna_path.split('/')[-1] ))

                    for batch in batches:
                        data_batch, label_batch = zip(*batch)
                        feed_dict={
                                model.input_x: data_batch,
                                model.input_y: label_batch,
                                model.dropout_keep_prob: 0.8,
                                model.batch_size: len(data_batch)
                                }
                        
                        train_step(feed_dict)
                        current_step = tf.train.global_step(sess, global_step)
                        if current_step % 100 == 0:
                            #valid_step(x_valid, y_valid)
                            path = saver.save(sess, checkpoint_prefix, global_step=current_step)
                            print("Saved model checkpoint to {}\n".format(path))


if __name__ == '__main__':
    film_vdna_files='./data/small_data/film.txt-train'
    commercial_vdna_files='./data/small_data/commercial.txt-train'
    train_model(film_vdna_files, commercial_vdna_files)
