/**
*  @file     http_courier.cpp
*  @brief    http通信类实现文件.
*  @see http_courier.h
*
*  @author   wangyinzhi
*  @date     2019/08/09
*
*  Change History : \n
*   -wangyingzhi 创建 2019/08/09
*/
#include "http_courier.h"
#include "TNLog.h"
#include "define.h"
#include "alg_handle.h"
#include "json/json.h"
#include "protocol_utils.h"

CHttpCourier::CHttpCourier()
{}

CHttpCourier::~CHttpCourier()
{}

CHttpCourier& CHttpCourier::GetInstance()
{
    static CHttpCourier g_httpCourier;
    return g_httpCourier;
}

void CHttpCourier::Init()
{
    //开始监听
    server_.Listen(g_config_.http_port);
    LOG_INFO_FMT("http listen port[%d] success", g_config_.http_port);

    // 绑定碰撞接口回调
    server_.Post("/study/lenet5/cpppy/train", std::bind(&CHttpCourier::TrainModelCB, this, std::placeholders::_1, std::placeholders::_2));
    server_.Post("/study/lenet5/cpppy/discern", std::bind(&CHttpCourier::DiscernCB, this, std::placeholders::_1, std::placeholders::_2));

    LOG_INFO("http courier init success");
}

void CHttpCourier::Stop()
{
    //开始监听
    server_.Stop();

    LOG_INFO("http courier stop success");
}
void CHttpCourier::TrainModelCB(HttpRequest& request, HttpResponse& response)
{
    std::stringstream ss;
    ss << request.GetStream().rdbuf();
    TrainModelReq req;
    if (!TrainReqParse(ss, req))
    {
        LOG_ERROR("train req pares failed");
        Response(BaseRspCompose(false), response);
        return ;
    }

    LOG_INFO("TrainModel response.");
    Response(BaseRspCompose(true), response);

    if(!CAlgHandle::GetInstance().TrainModel(req))
        LOG_INFO("TrainModel failed.");
    else
        LOG_INFO("TrainModel finished.");
}

void CHttpCourier::DiscernCB(HttpRequest& request, HttpResponse& response)
{
    std::stringstream ss;
    ss << request.GetStream().rdbuf();
    DiscernReq req;
    if (!DiscernReqParse(ss, req))
    {
        LOG_ERROR("train req pares failed");
        Response(BaseRspCompose(false), response);
        return;
    }

    DiscernRsp rsp;
    CAlgHandle::GetInstance().Discern(req, rsp);

    LOG_INFO("TrainModel response.");
    Response(DiscernReqCompose(rsp), response);
}

void CHttpCourier::Response(const std::string& body, HttpResponse& response)
{
    response.SetContentType("application/json");
    response.SetContentLength(body.length());
    response.SendBuffer(body.c_str(), body.length());
}
