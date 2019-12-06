# -*- coding:utf-8 -*-
import tensorflow as tf
import sys

# 网络输入输出
INPUT_NODE = 784
OUTPUT_NODE = 10
#图片形状
IMAGE_SZIE = 28
NUM_CHANNELS = 1
NUM_LABLES = 10
#第一次卷积形状
CONV1_DEEP = 32
CONV1_SIZE = 5
#第二次卷积形状
CONV2_DEEP = 64
CONV2_SIZE = 5
#全连接节点个数
FC_SIZE = 512

def self_print(info):
    ''' python接口中如果有print的调用，则C++调python接口失败，C++调python接口时，如果需要
    打印信息，需要依赖sys.stdout.write，为啥不知道，按理讲print底层也是调sys.stdout.write
    的，具体日后调查
    '''
    sys.stdout.write("%s\n" %(info))

def forward(input_tensor, train, regl):
    with tf.compat.v1.variable_scope('l1-conv1'):
        c1_w = tf.compat.v1.get_variable('weight', [CONV1_SIZE,CONV1_SIZE,NUM_CHANNELS, CONV1_DEEP], initializer=tf.truncated_normal_initializer(stddev=0.1))
        c1_b = tf.compat.v1.get_variable('bias', [CONV1_DEEP],initializer=tf.constant_initializer(0.0))
        # 定义的卷积层输入为28*28*l的原始图片像素。使用边长为5，深度为32的过滤器，
        # 过滤器移动的步长为1，且使用全0填充。所以输出为 28*28*32 的矩阵
        conv1 = tf.nn.conv2d(input_tensor, c1_w, strides=[1,1,1,1], padding='SAME')
        relu1 = tf.nn.relu(tf.nn.bias_add(conv1, c1_b))

    # 池化层，这里选用最大池化层，池化层过滤器的边长为 2, 使用全0填充且移动的步长为2
    with tf.compat.v1.variable_scope('l2-pool1'):
        pool1 = tf.nn.max_pool(relu1, ksize=[1,2,2,1], strides=[1,2,2,1], padding='SAME')

    with tf.compat.v1.variable_scope('l3-conv2'):
        c2_w = tf.compat.v1.get_variable('weight', [CONV2_SIZE, CONV2_SIZE, CONV1_DEEP, CONV2_DEEP], initializer=tf.truncated_normal_initializer(stddev=0.1))
        c2_b = tf.compat.v1.get_variable('bias', [CONV2_DEEP], initializer=tf.constant_initializer(0.0))
        conv2 = tf.nn.conv2d(pool1, c2_w, strides=[1,1,1,1], padding='SAME')
        relu2 = tf.nn.relu(tf.nn.bias_add(conv2, c2_b))

    with tf.compat.v1.variable_scope('l4_pool2'):
        pool2 = tf.nn.max_pool(relu2, ksize=[1,2,2,1], strides=[1,2,2,1], padding='SAME')

    # 将第四次池化输出7*7*64进行维度转换，即拉成一个向量，以进行第一次全连接
    pool_shape = pool2.get_shape().as_list()
    nodes = pool_shape[1] * pool_shape[2] * pool_shape[3]
    reshaped = tf.reshape(pool2, [pool_shape[0], nodes])

    with tf.compat.v1.variable_scope('l5-fc1'):
        fc1_w = tf.compat.v1.get_variable("weight", [nodes, FC_SIZE], initializer=tf.truncated_normal_initializer(stddev=0.1))
        fc1_b = tf.compat.v1.get_variable("bias", [FC_SIZE], initializer=tf.constant_initializer(0.1))

        # 正则存在时，将权重添加到losses
        if regl != None:
            tf.compat.v1.add_to_collection('losses', regl(fc1_w))

        fc1 = tf.nn.relu(tf.matmul(reshaped, fc1_w) + fc1_b)

        # 训练时，使用dropout以避免过拟合
        if train:
            fc1 = tf.nn.dropout(fc1, 0.5)

    with tf.variable_scope('l6_fc2'):
        fc2_w = tf.compat.v1.get_variable("weight", [FC_SIZE, NUM_LABLES], initializer=tf.truncated_normal_initializer(0.1))
        fc2_b = tf.compat.v1.get_variable("bias", [NUM_LABLES], initializer=tf.constant_initializer(0.1))

        if regl != None:
            tf.compat.v1.add_to_collection('losses', regl(fc2_w))

        output = tf.matmul(fc1, fc2_w) + fc2_b

    return output
