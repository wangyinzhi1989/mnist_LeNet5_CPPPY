/**
*  @file     protocol_utils.h
*  @brief    协议工具函数定义文件.
*
*  @author   wangyinzhi
*  @date     2019/04/10
*
*  Change History : \n
*   -wangyingzhi 创建 2019/04/10
*/

#ifndef TN_TSE_PROTOCOL_UTILS_H
#define TN_TSE_PROTOCOL_UTILS_H

#include <sstream>
#include "define.h"
#include "json/json.h"

/***********************协议解析***********************/
/**
*@brief 训练请求信息解析
*@param  req 请求信息
*@param  info 解析结果
*@return true-成功 false-失败
*/
bool TrainReqParse(const std::stringstream& req, TrainModelReq& res);

/**
*@brief 识别请求信息解析
*@param  req 请求信息
*@param  info 解析结果
*@return true-成功 false-失败
*/
bool DiscernReqParse(const std::stringstream& req, DiscernReq& res);

/***********************协议合成***********************/
/**
*@brief 基本应答信息合成
*@param  code 错误码
*@return 合成结果
*/
std::string BaseRspCompose(bool res);

/**
*@brief 识别应答信息合成
*@param  info 待合成信息
*@return 合成结果
*/
std::string DiscernReqCompose(DiscernRsp& info);

#endif // TN_TSE_PROTOCOL_UTILS_H
