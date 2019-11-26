/**
*  @file     main.cpp
*  @brief    服务器入口文件.
*
*  Change History : \n
*   -wangyingzhi 创建 2018/2/22
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "define.h"
#include "alg_handle.h"
#include "http_courier.h"
#include "sysutils.h"
#include "TNLog.h"

//! 退出互斥量
std::mutex g_exit_mutex_;
//! 退出条件变量
std::condition_variable g_exit_cv_;
//! 配置
Config g_config_;

static void Wait()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(g_exit_mutex_);
        if (std::cv_status::no_timeout == g_exit_cv_.wait_for(lock, std::chrono::milliseconds(2000)))
            return ;
    }
}

//! 程序入口
bool AppMain(int argc, char *argv[])
{
    // 系统初始化
    std::string exe_path = get_executable_dir();
    std::string logConfigPath = exe_path + "../conf/log.properties";
    TNLOG_INIT(logConfigPath);

    if (!CAlgHandle::GetInstance().Init(exe_path))
    {
        LOG_ERROR("alg handle init failed");
        return false;
    }

    CHttpCourier::GetInstance().Init();

    LOG_INFO("TargetSearcher init success");
    return true;
}

//! Main函数
int main(int argc, char* argv[])
{
    if (AppMain(argc, argv))
        Wait();

    CHttpCourier::GetInstance().Stop();
    LOG_DEBUG("进程结束");
    return 0;
}
