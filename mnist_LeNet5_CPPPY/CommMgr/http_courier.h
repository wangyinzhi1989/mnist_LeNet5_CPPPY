/**
*  @file     http_courier.h
*  @brief    http通信类定义文件.
*
*  @author   wangyinzhi
*  @date     2019/08/09
*
*  Change History : \n
*   -wangyingzhi 创建 2019/08/09
*/

#ifndef TN_TSE_HTTP_COURIER_H
#define TN_TSE_HTTP_COURIER_H

#include "define.h"
#include "http_server.h"
#include "http_request.h"
#include "http_response.h"
#include "sysutils.h"

using namespace tn_http;

/**
* @brief CHttpCourier类
* http通信类
**/
class CHttpCourier
{
public:
    //! 禁止拷贝
    CHttpCourier(const CHttpCourier&) = delete;
    CHttpCourier& operator=(const CHttpCourier&) = delete;

    /**
    *@brief 获取系统唯一实例
    *@param  void
    *@return 唯一实例
    */
    static CHttpCourier &GetInstance();

    /**
    *@brief 初始化
    *@param  void
    *@return void
    */
    void Init();

    /**
    *@brief 停止
    *@param  void
    *@return void
    */
    void Stop();

    /**
    *@brief 训练模型请求回调
    *@param  request 请求对象
    *@param  response 应答对象
    *@return void
    */
    void TrainModelCB(HttpRequest& request, HttpResponse& response);

    /**
    *@brief 识别请求回调
    *@param  type 管理类型
    *@param  request 请求对象
    *@param  response 应答对象
    *@return void
    */
    void DiscernCB(HttpRequest& request, HttpResponse& response);

private:
    CHttpCourier();
    ~CHttpCourier();

    /**
    *@brief 接口应答
    *@param  body 应答消息
    *@param  response 应答对象
    *@return void
    */
    void Response(const std::string& body, HttpResponse& response);

private:
    HttpServer server_;                                         //!< http 服务对象
};

#endif // TN_TSE_HTTP_COURIER_H
