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

//! 系统存活标志定义
bool g_sysLive = true;
//! 配置
Config g_config_;

static void Wait()
{
    while (g_sysLive)
    {

        char type = 'z';
        int id = -1;

        cin.sync();
        cin.clear();
        fflush(stdin);
        cin >> type;
        switch (type)
        {
        case 'Q':
        case 'q':
            g_sysLive = false;
            LOG_INFO("Manual exit!");
            break;
        default:
            break;
        }
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
