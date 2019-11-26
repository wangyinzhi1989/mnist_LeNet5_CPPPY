/**
*  @file     protocol_utils.cpp
*  @brief    协议工具函数实现文件.
*  @see protocol_utils.h
*
*  @author   wangyinzhi
*  @date     2019/04/10
*
*  Change History : \n
*   -wangyingzhi 创建 2019/04/10
*/
#include <algorithm>
#include "protocol_utils.h"
#include "sysutils.h"
#include "TNLog.h"

//! 宏定义json解析共通
#define JSON_PARSE                                  \
    Json::Reader reader;                            \
    Json::Value root;                               \
    if (!reader.parse(req.str(), root))             \
    { LOG_ERROR(req.str().c_str());                 \
    return false; }                           \

bool TrainReqParse(const std::stringstream& req, TrainModelReq& res)
{
    JSON_PARSE

        try
    {
        if (!root["steps"].isInt())
            return false;

        // @note 这里和对方约定好，类型的表示后直接使用强转赋值，后期修改时要一起修改
        res.training_steps = root["steps"].asInt();
        res.rate_base = root["rate_base"].asFloat();
        res.rate_decay = root["rate_decay"].asFloat();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool DiscernReqParse(const std::stringstream& req, DiscernReq& res)
{
    JSON_PARSE

    try
    {
        if (!root["img_path"].isString())
            return false;

        res.img_path = root["img_path"].asString();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

std::string BaseRspCompose(bool res)
{
    Json::Value root;
    root["res"] = res;
    Json::StyledWriter swriter;
    return swriter.write(root);
}

std::string DiscernReqCompose(DiscernRsp& info)
{
    Json::Value root;
    root["number"] = info.number;
    root["err_msg"] = info.err_msg;
    Json::StyledWriter swriter;
    return swriter.write(root);
}
