/**
*  @file     engine.h
*  @brief    引擎类定义文件.
*
*  @author   wangyinzhi
*  @date     2019/08/09
*
*  Change History : \n
*   -wangyingzhi 创建 2019/08/09
*/

#ifndef TN_TSE_ENGINE_H
#define TN_TSE_ENGINE_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "NvInfer.h"

using namespace nvinfer1;

/**
* @brief CEngine类
* 引擎类
**/
class CEngine
{
public:
    CEngine();
    ~CEngine();

    /**
    *@brief 加载模型
    *@param  model 模型
    *@return true-成功，false-失败
    */
    bool Init(std::string& model);

    /**
    *@brief 加载uff模型并保存引擎
    *@param  uff_model uff模型
    *@return true-成功，false-失败
    */
    bool LoadUffAndSave(std::string& uff_model);

    /**
    *@brief 推理
    *@param  imgs 待检测图片
    *@param  label 检测结果
    *@return true-成功，false-失败
    */
    bool Inference(std::vector<cv::Mat>& imgs, std::vector<int>& label);

    int GetMaxBatchSize()
    {
        return max_batch_size_;
    }

private:
    /**
    *@brief 构建显存
    *@param  void
    *@return void
    */
    void BuildBuf();

    /**
    *@brief 修整图片
    *@param  in 输入的图片
    *@param  out 修整后的图片
    *@return void
    */
    void TrimImg(std::vector<cv::Mat>& in, std::vector<cv::Mat>& out);

private:
    ICudaEngine* engine_;                           //!< TR引擎
    IExecutionContext* context_;                    //!< TR上下文
    cudaStream_t cuda_stream_;                      //!< cuda流
    int max_batch_size_;                            //!< 引擎最大批处理量
    void* input_gup_bufs_[2];                       //!< 输入的显存缓存
    void* output_buf_{ nullptr };                   //!< 输出的缓存
    Dims3 input_dims_{ 1,28,28 };                   //!< 输入的维度
    Dims3 output_dims_{ 10,1,1 };                     //!< 输入的维度
    cv::Size input_size_{ 28,28 };                  //!< 输入的尺寸
    int in_size_{ 1 };                              //!< 输入单个缓存大小
    int out_size_{ 1 };                             //!< 输出单个缓存大小
};

#endif // TN_TSE_ENGINE_H
