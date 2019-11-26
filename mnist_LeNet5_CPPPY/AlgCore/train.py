# -*- coding:utf-8 -*-
import tensorflow as tf
from tensorflow.examples.tutorials.mnist import input_data
import forward
import os

BATCH_SIZE = 100                # 批大小
#LEARING_RATE_BASE = 0.2         # 最初学习率
#LEARING_RATE_DECAY = 0.9       # 学习速率衰减率
REGULARAZTION_RATE = 0.0001     # 正则化系数
MOVING_AVERAGE_DECAY = 0.99     # 滑动平均衰减率

def train(model_path_b, steps, rate_base, rate_decay):
    forward.self_print(type(model_path_b))
    #C++ 传入的是bytes类型的参数，需要解码成string类型的结果
    model_path = model_path_b.decode()
    forward.self_print("model_path[%s] steps[%d] rate_base[%g] rate_decay[%g]" % (model_path, steps, rate_base, rate_decay))
    
    mnist = input_data.read_data_sets("/home/wangyinzhi/study/data/mnist/train", one_hot=True)
    # 定义输入输出占位
    x = tf.placeholder(tf.float32, [BATCH_SIZE, forward.IMAGE_SZIE, forward.IMAGE_SZIE, forward.NUM_CHANNELS], name='x-input')
    y_ = tf.placeholder(tf.float32, [None, forward.OUTPUT_NODE], name='y-input')

    forward.self_print("num_examples[%d]" % mnist.train.num_examples)

    # 正则公式
    regl = tf.contrib.layers.l2_regularizer(REGULARAZTION_RATE)

    # 前向传播
    y = forward.forward(x, True, regl)
    # 训练次数
    global_step = tf.Variable(0, trainable=False)

    # 创建滑动平均值计算器
    var_averages = tf.train.ExponentialMovingAverage(MOVING_AVERAGE_DECAY, global_step)
    var_averages_op = var_averages.apply(tf.trainable_variables())

    # 计算损失值 
    '''tf.argmax(vector, 1)：返回的是vector中的最大值的索引号，如果vector是一个向量，
        那就返回一个值，如果是一个矩阵，那就返回一个向量，这个向量的每一个维度都是
        相对应矩阵行的最大值元素的索引号。
    '''
    cross_entropy = tf.nn.sparse_softmax_cross_entropy_with_logits(labels=tf.argmax(y_, 1), logits=y)
    cross_entropy_mean = tf.reduce_mean(cross_entropy)
    loss = cross_entropy_mean + tf.add_n(tf.get_collection('losses'))

    """ 计算学习率 exponential_decay(learning_rate 初始学习率,global_step 当前迭代次数, 
        decay_steps 衰减速度, decay_rate 学习率衰减系数通常介于0-1之间)
        learning_rate = learning_rate * decay_rate ^ (global_step / decay_steps) 
    """
    learning_rate = tf.train.exponential_decay(rate_base, global_step,
                                              mnist.train.num_examples/ BATCH_SIZE,
                                              rate_decay, staircase=True)

    # 训练
    train_step = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss, global_step = global_step)
    """tf.control_dependencies是tensorflow中的一个flow顺序控制机制，作用有二：插入依赖（dependencies）和清空依赖（依赖是op或tensor）。常见的tf.control_dependencies是tf.Graph.control_dependencies的装饰器，它们用法是一样的。
        https://blog.csdn.net/hustqb/article/details/83545310
        这里: 在执行train_op之前会先执行[train_step, variable_averages_op]
    """
    with tf.control_dependencies([train_step, var_averages_op]):
        # 空的执行 什么也不做。仅用作控制边的占位符。
        train_op = tf.no_op(name = 'train')

    # 模型保存器
    saver = tf.train.Saver()
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    with tf.Session(config = config) as sess:
        tf.global_variables_initializer().run()
        forward.self_print("training start")

        # 训练时不再使用验证数据来测试模型，验证和测试通过eval来完成
        for i in range(steps):
            xs,ys = mnist.train.next_batch(BATCH_SIZE)
            x_img = xs.reshape(BATCH_SIZE, forward.IMAGE_SZIE, forward.IMAGE_SZIE, forward.NUM_CHANNELS)

            _,loss_value, step, rate, cross = sess.run([train_op, loss, global_step, learning_rate, cross_entropy_mean], feed_dict={x:x_img, y_:ys})

            if i % 1000 == 0:
                # 训练情况输出，只输出模型在当前训练batch上的损失函数大小
                forward.self_print("training strp[%d] loss [%g] rate[%g] cross[%g]" % (step, loss_value, rate, cross))
                # 保存当前模型
                saver.save(sess, model_path, global_step=global_step)

        forward.self_print("training end")
