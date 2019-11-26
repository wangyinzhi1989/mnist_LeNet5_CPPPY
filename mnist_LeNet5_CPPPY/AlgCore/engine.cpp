/**
*  @file     alg_handle.cpp
*  @brief    算法操作类实现文件.
*  @see alg_handle.h
*
*  @author   wangyinzhi
*  @date     2019/08/09
*
*  Change History : \n
*   -wangyingzhi 创建 2019/08/09
*/

#include "TNLog.h"
#include "engine.h"
#include "NvUffParser.h"
#include "logger.h"
#include "define.h"
#include <cuda_runtime_api.h>
#include <fstream>
#include <opencv2/dnn/dnn.hpp>

using namespace nvuffparser;

CEngine::CEngine()
    : engine_(nullptr)
    , context_(nullptr)
    , cuda_stream_(nullptr)
    , max_batch_size_(0)
{}

CEngine::~CEngine()
{
    if (engine_)
    {
        engine_->destroy();
        engine_ = nullptr;
    }

    if (context_)
    {
        context_->destroy();
        context_ = nullptr;
    }

    if (cuda_stream_)
    {
        cudaStreamDestroy(cuda_stream_);
        cuda_stream_ = nullptr;
    }

    if (input_gup_bufs_[0])
    {
        // 释放显存
        cudaFree(input_gup_bufs_[0]);
        input_gup_bufs_[0] = nullptr;
    }

    if (input_gup_bufs_[1])
    {
        // 释放显存
        cudaFree(input_gup_bufs_[1]);
        input_gup_bufs_[1] = nullptr;
    }

    if (output_buf_)
    {
        // 释放零拷贝内存
        cudaFreeHost(output_buf_);
        output_buf_ = nullptr;
    }
}

bool CEngine::Init(std::string& model)
{
    /*std::string aa = "/home/wangyinzhi/study/TensorFlow/mnist/model/mnist_LeNet5_CPPPY.uff";
     LoadUffAndSave(aa);
     return true;*/
    if (cudaSuccess != cudaStreamCreateWithFlags(&cuda_stream_, cudaStreamNonBlocking))
    {
        LOG_ERROR("cuda stream create failed.");
        return false;
    }

    IRuntime* runtime = createInferRuntime(CLogger::GetInstance());
    std::stringstream model_stream;
    std::ifstream file_stream(model, std::ios::ate);
    auto size = file_stream.tellg();
    file_stream.seekg(0);
    void* model_mem = malloc(size);
    file_stream.read(static_cast<char*>(model_mem), size);
    engine_ = runtime->deserializeCudaEngine(model_mem, size, nullptr);
    free(model_mem);
    if (NULL == engine_)
    {
        LOG_ERROR_FMT("engine deserialize failed. file[%s]", model.c_str());
        return false;
    }
    max_batch_size_ = engine_->getMaxBatchSize();
    LOG_INFO_FMT("engine max batch size[%d]", max_batch_size_);

    context_ = engine_->createExecutionContext();
    if (nullptr == context_)
    {
        LOG_ERROR("createExecutionContext failed.");
        engine_->destroy();
        return false;
    };

    BuildBuf();

    LOG_INFO_FMT("load engine[%s] max batch[%d] success.", model.c_str(), max_batch_size_);
    return true;
}

bool CEngine::LoadUffAndSave(std::string& uff_model)
{
    if (nullptr != engine_)
    {
        engine_->destroy();
        engine_ = nullptr;
    }

    // 创建uff 解析器，并注册输入输出
    IUffParser* parser = createUffParser();
    parser->registerInput("x-input", input_dims_, UffInputOrder::kNCHW);
    parser->registerOutput("inference");
    /* parser->registerInput("in", input_dims_, UffInputOrder::kNCHW);
    parser->registerOutput("out");*/

    // 创建构造器
    IBuilder* builder = createInferBuilder(CLogger::GetInstance());
    if (nullptr == builder)
    {
        LOG_ERROR("createInferBuilder failed");
        return false;
    }

    //uff_model = "/home/wangyinzhi/tensorflow_build/TensorRT-5.1.5.0/data/mnist/lenet5.uff";
    // 创建网络
    INetworkDefinition* network = builder->createNetwork();
    if (!parser->parse(uff_model.c_str(), *network, DataType::kFLOAT))
    {
        LOG_ERROR_FMT("parse uff model failed. uff[%s]", uff_model.c_str());
        network->destroy();
        builder->destroy();
        return false;
    }
    max_batch_size_ = 1;
    // 打开半精度
    //builder->setFp16Mode(true);
    builder->setMaxBatchSize(max_batch_size_);
    builder->setMaxWorkspaceSize(1<<30);
    //builder->setInt8Mode(false);

    // 使用DLA硬件加速器
    /*builder->allowGPUFallback(true);
    builder->setDefaultDeviceType(DeviceType::kDLA);
    int dla_num = builder->getNbDLACores();
    LOG_DEBUG_FMT("dla num[%d]", dla_num);
    builder->setDLACore(dla_num-1);
    builder->setStrictTypeConstraints(true);*/

    // 创建引擎
    engine_ = builder->buildCudaEngine(*network);
    if (nullptr == engine_)
    {
        LOG_ERROR("buildCudaEngine failed.");
        network->destroy();
        builder->destroy();
        return false;
    };

    context_ = engine_->createExecutionContext();
    if (nullptr == context_)
    {
        LOG_ERROR("createExecutionContext failed.");
        network->destroy();
        builder->destroy();
        engine_->destroy();
        return false;
    };

    BuildBuf();

    IHostMemory *model_memory = engine_->serialize();
    if (NULL == model_memory || NULL == model_memory->data())
    {
        LOG_ERROR("engine serialize failed.");
        return false;
    }

    std::string model_file = g_config_.model_path + g_config_.tr_model_file;

    /*std::stringstream gie_model_stream;
    gie_model_stream.seekg(0, gie_model_stream.beg);
    gie_model_stream.write(static_cast<const char*>(model_stream->data()), model_stream->size());*/
    std::ofstream out_file;
    out_file.open(model_file);
    out_file.write(static_cast<const char*>(model_memory->data()), model_memory->size());
    //out_file << gie_model_stream.rdbuf();
    out_file.close();

    model_memory->destroy();
    network->destroy();
    builder->destroy();

    return true;
}


inline void readPGMFile(const std::string& fileName, uint8_t* buffer, int inH, int inW)
{
    std::ifstream infile(fileName, std::ifstream::binary);
    assert(infile.is_open() && "Attempting to read from a file that is not open.");
    std::string magic, h, w, max;
    infile >> magic >> h >> w >> max;
    infile.seekg(1, infile.cur);
    infile.read(reinterpret_cast<char*>(buffer), inH * inW);
}

bool CEngine::inference(std::vector<cv::Mat>& imgs, std::vector<int>& label)
{
    if (!engine_)
    {
        LOG_WARN("engine is null");
        return false;
    }

    // 创建推理上下文
    IExecutionContext* context = engine_->createExecutionContext();
    int batch_size = imgs.size();

    // 批超过32时，不处理
    if (batch_size > max_batch_size_)
    {
        LOG_WARN_FMT("batch[%d] max [%d]", batch_size, max_batch_size_);
        return false;
    }

    // 图片整理
    std::vector<cv::Mat> engine_imgs;
    TrimImg(imgs, engine_imgs);
    //input_mat_ = cv::dnn::blobFromImages(engine_imgs, 1.0, input_size_, cv::Scalar(), false);
    input_mat_ = engine_imgs[0];

    int64_t eltCount = 28 * 28;
    float* inputs = new float[eltCount];
    /*uint8_t fileData[eltCount];
    readPGMFile("/home/wangyinzhi/study/data/mnist/test/0.pgm", fileData, 28,28);*/
    std::cout << "Input:" << std::endl;
    for (int i = 0; i < eltCount; i++)
    {
        //std::cout << (" .:-=+*#%@"[input_mat_.data[i] / 26]) << (((i + 1) % 28) ? "" : "\n");
        std::cout << (input_mat_.data[i]==0x01 ? " ":"#") << (((i + 1) % 28) ? "" : "\n");
        inputs[i] = input_mat_.data[i];
    }
    std::cout << std::endl;

    // 拷贝图片到显存
    //cudaError res = cudaMemcpy(input_gup_bufs_[0], inputs, batch_size*in_size_ * sizeof(float), cudaMemcpyHostToDevice);
    cudaError res = cudaMemcpyAsync(input_gup_bufs_[0], inputs, batch_size*in_size_ * sizeof(float), cudaMemcpyHostToDevice, cuda_stream_);
    if (cudaSuccess != res)
    {
        LOG_ERROR_FMT("input buf host to device failed. err[%d  %s]", res, cudaGetErrorString(res));
        return false;
    }
    delete[] inputs;

    // 推理，通知事件置空，做成同步的
    context_->enqueue(batch_size, &input_gup_bufs_[0], cuda_stream_, nullptr);
    //context_->execute(batch_size, &input_gup_bufs_[0]);

    // 拷贝结果到内存
    //res = cudaMemcpy(output_buf_, input_gup_bufs_[1], batch_size*out_size_ * sizeof(float), cudaMemcpyDeviceToHost);
    res = cudaMemcpyAsync(output_buf_, input_gup_bufs_[1], batch_size*out_size_ * sizeof(float), cudaMemcpyDeviceToHost, cuda_stream_);
    if (cudaSuccess != res)
    {
        LOG_ERROR_FMT("out buf device to host failed. err[%d  %s]", res, cudaGetErrorString(res));
        return false;
    }

    // 同步等待gpu执行完成
    res = cudaStreamSynchronize(cuda_stream_);
    if (cudaSuccess != res)
    {
        LOG_ERROR_FMT("out buf device to host failed. err[%d  %s]", res, cudaGetErrorString(res));
        return false;
    }

    // 取结果
    int tmp_label = -1;
    float tmp_max = 0;
    int blob_size = out_size_ * sizeof(float);
    float f_res[out_size_];
    std::string tmp_res_log;
    for (int i = 0; i < batch_size; ++i)
    {
        tmp_label = -1;
        tmp_max = 0;
        tmp_res_log.clear();
        memset(&f_res[0], 0, blob_size);

        float* tmp_res = static_cast<float*>(output_buf_ + i * blob_size);
        memcpy(&f_res[0], tmp_res, blob_size);
        for (int j = 0; j < out_size_; ++j)
        {
            tmp_res_log += std::to_string(f_res[j]);
            tmp_res_log += " ";
            if (tmp_max < f_res[j])
            {
                tmp_max = f_res[j];
                tmp_label = j;
            }
        }
        LOG_INFO_FMT("img index[%d] label[%d] inference[%s]", i, tmp_label, tmp_res_log.c_str());
        label.push_back(tmp_label);
    }
    return true;
}

//! 比较dims，仅比较维度大小，不比较维度类型
bool ComperDims(Dims& d1, Dims& d2)
{
    if (d1.nbDims != d2.nbDims)
        return false;

    for (int i = 0; i<d1.nbDims;++i)
    {
        if (d1.d[i] != d2.d[i])
            return false;
    }

    return true;
}

void CEngine::BuildBuf()
{
    if (input_gup_bufs_[0])
    {
        // 释放显存
        cudaFree(input_gup_bufs_[0]);
        input_gup_bufs_[0] = nullptr;
    }

    if (input_gup_bufs_[1])
    {
        // 释放显存
        cudaFree(input_gup_bufs_[1]);
        input_gup_bufs_[1] = nullptr;
    }

    if (output_buf_)
    {
        // 释放零拷贝内存
        cudaFreeHost(output_buf_);
        output_buf_ = nullptr;
    }

    // 获取绑定索引的数目
    int nb_bindings = engine_->getNbBindings();
    if (2 != nb_bindings)
    {
        LOG_ERROR("binding index not equal to 2");
        abort();
    }

    int in_idx = 0, out_idx = 0, i = 0;
    for (; i < nb_bindings; ++i)
    {
        if (engine_->bindingIsInput(i))
            in_idx = i;
        else
            out_idx = i;
    }

    Dims in_dims = std::move(engine_->getBindingDimensions(in_idx));
    if (!ComperDims(input_dims_, in_dims))
    {
        LOG_ERROR("input dims error");
        abort();
    }
    std::string in_dims_log("in dims(");
    std::for_each(&(in_dims.d[0]), &(in_dims.d[in_dims.nbDims]), [&](int v) {
        in_dims_log += std::to_string(v);
        in_dims_log += ",";
        in_size_ *= v; });
    in_dims_log.pop_back();
    in_dims_log += ")";
    LOG_INFO_STM(in_dims_log);

    Dims out_dims = std::move(engine_->getBindingDimensions(out_idx));
    if (!ComperDims(output_dims_, out_dims))
    {
        LOG_ERROR("output dims error");
        abort();
    }
    std::string out_dims_log("out dims(");
    std::for_each(&(out_dims.d[0]), &(out_dims.d[out_dims.nbDims]), [&](int v) {
        out_dims_log += std::to_string(v);
        out_dims_log += ",";
        out_size_ *= v; });
    out_dims_log.pop_back();
    out_dims_log += ")";
    LOG_INFO_STM(out_dims_log);

    cudaError res = cudaMalloc(&(input_gup_bufs_[0]), max_batch_size_*in_size_ * sizeof(float));
    if (cudaSuccess != res)
    {
        LOG_ERROR_FMT("cuda malloc input buf failed. error[%d  %s]", res, cudaGetErrorString(res));
        abort();
    }
    res = cudaMalloc(&(input_gup_bufs_[1]), max_batch_size_*out_size_ * sizeof(float));
    if (cudaSuccess != res)
    {
        LOG_ERROR_FMT("cuda malloc output buf failed. error[%d  %s]", res, cudaGetErrorString(res));
        abort();
    }
    cudaMallocHost(&output_buf_, max_batch_size_*out_size_ * sizeof(float));
    if (cudaSuccess != res)
    {
        LOG_ERROR_FMT("cuda malloc host output buf failed. error[%d  %s]", res, cudaGetErrorString(res));
        abort();
    }
}

void CEngine::TrimImg(std::vector<cv::Mat>& in, std::vector<cv::Mat>& out)
{
    int i = 0, size = in.size();
    out.resize(size);
    cv::Mat tmp;
    std::vector<cv::Mat> split_mat;
    for (; i < size; ++i)
    {
        split_mat.clear();
        // 归一化处理 范围法
        cv::normalize(in[i], tmp, 1.0, 0.0, cv::NORM_MINMAX);

        // 重置大小, 双线性插值法
        cv::resize(tmp, tmp, input_size_, 0.0, 0.0, cv::INTER_LINEAR);

        // 拆分维度
        cv::split(tmp, split_mat);
        out[i] = std::move(split_mat[0]);
    }
}
