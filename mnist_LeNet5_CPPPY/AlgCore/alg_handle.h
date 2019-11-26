/**
*  @file     alg_handle.h
*  @brief    算法操作类定义文件.
*
*  @author   wangyinzhi
*  @date     2019/08/09
*
*  Change History : \n
*   -wangyingzhi 创建 2019/08/09
*/

#ifndef TN_TSE_ALG_HANDLE_H
#define TN_TSE_ALG_HANDLE_H

#include <functional>
#include <atomic>
#include <mutex>
#include "define.h"
#include "Python.h"
#include "engine.h"

/**
* @brief CAlgHandle类
* 算法操作类
**/
class CAlgHandle 
{
public:
    //! 禁止拷贝
    CAlgHandle(const CAlgHandle&) = delete;
    CAlgHandle& operator=(const CAlgHandle&) = delete;

    /**
    *@brief 获取系统唯一实例
    *@param  void
    *@return 唯一实例
    */
    static CAlgHandle &GetInstance();

    /**
    *@brief 初始化
    *@param  exe_path 可执行文件路径
    *@return true-成功 false-失败
    */
    bool Init(std::string& exe_path);

    /**
    *@brief 训练模型
    *@param  info 参数
    *@return true-成功 false-失败
    */
    bool TrainModel(TrainModelReq& info);

    /**
    *@brief 识别
    *@param  info 参数
    *@param  rsp 识别结果
    *@return void
    */
    void Discern(DiscernReq& req, DiscernRsp& rsp);

private:
    CAlgHandle();
    ~CAlgHandle();

    /**
    *@brief python 函数载入
    *@param  exe_path 可执行文件路径
    *@return true-成功 false-失败
    */
    bool PyFunLoad(std::string& exe_path);

    /**
    *@brief 训练
    *@param  void
    *@return void
    */
    void Train();

    /**
    *@brief 验证
    *@param  void
    *@return void
    */
    void Eval();

    /**
    *@brief 重新加载算法对象
    *@param  modle 最优模型
    *@return true-成功 false-失败
    */
    bool Test(std::string& modle);

private:
    PyObject *py_train_fun_{nullptr};                   //!< 训练函数对象
    PyObject *py_test_fun_{ nullptr };                  //!< 测试函数对象
    PyObject *py_eval_fun_{ nullptr };                  //!< 验证函数对象
    PyObject *py_stop_fun_{ nullptr };                  //!< 停止验证函数对象
    std::string exe_path_;                              //!< 可执行文件路径
    TrainModelReq info_;                                //!< 训练请求信息
    std::atomic_bool train;                             //!< 训练成功标志
    std::atomic_bool eval;                              //!< 验证成功标志

    CEngine engine_;                                    //!< 推理引擎
};

#endif // TN_TSE_ALG_HANDLE_H
