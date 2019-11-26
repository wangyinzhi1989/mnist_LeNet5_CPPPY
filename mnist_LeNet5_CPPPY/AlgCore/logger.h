/**
*  @file     logger.h
*  @brief    tensorrt log封装类定义文件.
*
*  @author   wangyinzhi
*  @date     2019/08/09
*
*  Change History : \n
*   -wangyingzhi 创建 2019/08/09
*/

#ifndef TN_TSE_LOGGER_H
#define TN_TSE_LOGGER_H

#include <iostream>
#include "NvInfer.h"
#include "TNLog.h"

/**
* @brief CLogger类
* tensorrt log封装类
**/
class CLogger : public ILogger
{
public:
    //! 禁止拷贝
    CLogger(const CLogger&) = delete;
    CLogger& operator=(const CLogger&) = delete;
    CLogger() {}
    ~CLogger() {}

    //! 单例
    static CLogger& GetInstance()
    {
        static CLogger g_logger;
        return g_logger;
    }

    //! 继承虚接口的实现
    void log(Severity severity, const char* msg) override
    {
        // suppress info-level messages
        if (severity != Severity::kINFO)
            std::cout << msg << std::endl;

        switch (severity)
        {
        case nvinfer1::ILogger::Severity::kINTERNAL_ERROR:
            LOG_ERROR(msg);
            break;
        case nvinfer1::ILogger::Severity::kERROR:
            LOG_ERROR(msg);
            break;
        case nvinfer1::ILogger::Severity::kWARNING:
            LOG_WARN(msg);
            break;
        case nvinfer1::ILogger::Severity::kINFO:
            LOG_INFO(msg);
            break;
        case nvinfer1::ILogger::Severity::kVERBOSE:
            LOG_DEBUG(msg);
            break;
        default:
            LOG_ERROR_FMT("tensorrt log type error. msg[%s]", msg);
            break;
        }
    }
};

#endif // TN_TSE_LOGGER_H
