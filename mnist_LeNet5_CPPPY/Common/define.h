/**
*  @file     define.h
*  @brief    共用定义文件.
*
*  @author   wangyinzhi
*  @date     2019/03/18
*
*  Change History : \n
*   -wangyingzhi 创建 2019/03/18
*/

#ifndef TN_TSE_DEFINE_H
#define TN_TSE_DEFINE_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <condition_variable>

//! 系统存活标志定义
extern bool g_sysLive;

//! 配置
typedef struct _config 
{
    int http_port;                                      //!< 人脸资产id
    std::string model_path;                             //!< 模型文件路径
    std::string model_name;                             //!< 模型文件名
    std::string pb_model_file;                          //!< pb模型文件名
    std::string uff_model_file;                         //!< uff模型文件名
    std::string tr_model_file;                          //!< Tensorrt模型文件名

    _config()
    {
        http_port = 9990;
        model_path = "/home/wangyinzhi/study/TensorFlow/mnist/model/";
        model_name = "model.ckpt";
        pb_model_file = "mnist_LeNet5_CPPPY.pb";
        uff_model_file = "mnist_LeNet5_CPPPY.uff";
        tr_model_file = "mnist_LeNet5_CPPPY.engine";
    }
}Config;
//! 配置声明
extern Config g_config_;

//! 训练请求信息
typedef struct _trainModelReq {
    int training_steps{ 0 };                            //!< 训练步数
    float rate_base{ 0.0 };                             //!< 最初学习率
    float rate_decay{ 0.0 };                            //!< 学习速率衰减率
}TrainModelReq;

//! 识别请求信息
typedef struct _discernReq {
    std::string img_path;                               //!< 识别图片地址
}DiscernReq;

//! 识别应答信息
typedef struct _discernRsp {
    int number{ -1 };                                   //!< 识别出的数字结果，-1时表示失败
    std::string err_msg;                                //!< 错误信息
}DiscernRsp;

#endif // TN_TSE_DEFINE_H
