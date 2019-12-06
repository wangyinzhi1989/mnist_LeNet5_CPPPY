# -*- coding: utf-8 -*-
import time
import tensorflow as tf
from tensorflow.examples.tutorials.mnist import input_data
# 加载forward中的常量和信息
import forward
import train
import uff
from pathlib import Path
from tensorflow.python.framework import graph_io
import os
from tensorflow.python.tools import freeze_graph

def to_uff(model_path, pb_model_file, uff_model_file):
    forward.self_print("to uff[%s]" % (model_path))
    with tf.Graph().as_default() as g:
        # 定义输入占位
        #x = tf.placeholder(tf.float32, [None, forward.INPUT_NODE], name='x-input')
        x = tf.compat.v1.placeholder(tf.float32, [32, forward.NUM_CHANNELS, forward.IMAGE_SZIE, forward.IMAGE_SZIE], name='x-input')

        # 因输入的是NCHW 需转成NHWC
        x_image = tf.transpose(x,[0,2,3,1])

        # 前向传播，因验证时无需关注正则化损失值
        y = forward.forward(x_image, False, None)
        inference = tf.add(x=0.0, y=y, name='inference')

        """ 通过变量重命名的方式来加载模型，这样在前向传播的过程中就不需要调用滑动
            平均的函数获取平均值了。这样就可以完全共用前向传播过程了。
            通过使用variables_to_restore函数，可以使在加载模型的时候将影子变量直接
            映射到变量的本身，所以我们在获取变量的滑动平均值的时候只需要获取到变量
            的本身值而不需要去获取影子变量
        """
        var_averages = tf.compat.v1.train.ExponentialMovingAverage(train.MOVING_AVERAGE_DECAY)
        var_to_restore = var_averages.variables_to_restore()
        saver = tf.compat.v1.train.Saver(var_to_restore)

        config = tf.compat.v1.ConfigProto()
        config.gpu_options.allow_growth = True
        with tf.Session(config=config) as sess:
            if model_path:
                saver.restore(sess, model_path)
                forward.self_print("pb_model_file[%s]" % (pb_model_file))
                [save_path, save_file] = os.path.split(pb_model_file)
                graph_io.write_graph(sess.graph, save_path, "tmp_graph.pb")
                freeze_graph.freeze_graph(save_path + "/tmp_graph.pb", '', False,
                                            model_path, "inference", 
                                            "save/restore_all",
                                            "save/Const:0", pb_model_file,
                                            False, "")

                # 转成uff
                uff.from_tensorflow_frozen_model(pb_model_file, output_nodes=[], preprocessor=None, input_node=[], quiet=False, text=False, list_nodes=False, output_filename=uff_model_file, write_preprocessed=False, debug_mode=False)

def testing(model_path_b, pb_model_file_b, uff_model_file_b):
    #C++ 传入的是bytes类型的参数，需要解码成string类型的结果
    model_path = model_path_b.decode()
    forward.self_print("model_path[%s]" % (model_path))
    pb_model_file = pb_model_file_b.decode()
    uff_model_file = uff_model_file_b.decode()
    mnist = input_data.read_data_sets("/home/wangyinzhi/study/data/mnist/train", one_hot=True)

    with tf.Graph().as_default() as g:
        # 定义输入输出占位
        x = tf.compat.v1.placeholder(tf.float32, [None, forward.INPUT_NODE], name='x-input')
        y_ = tf.compat.v1.placeholder(tf.float32, [None, forward.OUTPUT_NODE], name='y-input111')
        test_feed = {x:mnist.test.images, y_:mnist.test.labels}

        x_image = tf.reshape(x,[mnist.test.images.shape[0], forward.IMAGE_SZIE, forward.IMAGE_SZIE, forward.NUM_CHANNELS])

        # 前向传播，因验证时无需关注正则化损失值
        y = forward.forward(x_image, False, None)
        label = tf.add(x=0.0, y=y, name='label')
        correct_prediction = tf.equal(tf.argmax(y,1), tf.argmax(y_, 1))
        accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32), name='accuracy')

        """ 通过变量重命名的方式来加载模型，这样在前向传播的过程中就不需要调用滑动
            平均的函数获取平均值了。这样就可以完全共用前向传播过程了。
            通过使用variables_to_restore函数，可以使在加载模型的时候将影子变量直接
            映射到变量的本身，所以我们在获取变量的滑动平均值的时候只需要获取到变量
            的本身值而不需要去获取影子变量
        """
        var_averages = tf.compat.v1.train.ExponentialMovingAverage(train.MOVING_AVERAGE_DECAY)
        var_to_restore = var_averages.variables_to_restore()
        saver = tf.compat.v1.train.Saver(var_to_restore)

        forward.self_print("testing start")
        config = tf.compat.v1.ConfigProto()
        config.gpu_options.allow_growth = True
        with tf.Session(config=config) as sess:
            if model_path:
                saver.restore(sess, model_path)
                accuracy_score, x1, y1, y_1,label_1= sess.run([accuracy, x, y, y_, label], feed_dict= test_feed)
                forward.self_print("x shape:{0}".format(x1.shape))
                forward.self_print("y shape:{0}".format(y1.shape))
                forward.self_print("y_ shape:{0}".format(y_1.shape))
                forward.self_print("label_1 shape:{0}".format(label_1.shape))
                forward.self_print("label_1 value{0}".format(label_1[0]))
                forward.self_print("test accuracy_score:%g" % (accuracy_score))
                # 得分大于0.98就生成新的模型
                if accuracy_score > 0.90 and pb_model_file and uff_model_file:
                    to_uff(model_path, pb_model_file, uff_model_file);
            else:
                forward.self_print("model file[%s] not find" % (model_path))
        forward.self_print("testing end")
