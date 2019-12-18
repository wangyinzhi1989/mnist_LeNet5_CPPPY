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

#include <thread>
#include <opencv2/opencv.hpp>
#include "TNLog.h"
#include "alg_handle.h"
#include "py_thread_state_lock.h"
#include "sysutils.h"

CAlgHandle::CAlgHandle()
{
    wchar_t *b = L"/root/.pyenv/versions/anaconda3-5.1.0/bin:/root/.pyenv/versions/anaconda3-5.1.0/lib/python3.6/lib-dynload:/root/.pyenv/versions/anaconda3-5.1.0/lib/python3.6:/root/.pyenv/versions/anaconda3-5.1.0/lib/python3.6/site-packages:/root/.pyenv/versions/anaconda3-5.1.0/lib";
    Py_SetPythonHome(b);
    // python 环境初始化
    Py_Initialize();

    wchar_t *b1 = Py_GetPythonHome();
    if (NULL != b1)
        LOG_INFO_FMT("python home[%ls]", b1);
    // 线程支持初始化
    PyEval_InitThreads();
}

CAlgHandle::~CAlgHandle()
{
    Py_Finalize();
}

CAlgHandle& CAlgHandle::GetInstance()
{
    static CAlgHandle g_algHandle;
    return g_algHandle;
}

bool CAlgHandle::Init(std::string& exe_path)
{
    exe_path_ = exe_path;
    std::string path = g_config_.model_path + g_config_.tr_model_file;
    if (!engine_.Init(path))
    {
        LOG_ERROR("CAlgHandle init failed");
        return false;
    }

    LOG_INFO("CAlgHandle init success");
    return true;
}

bool CAlgHandle::TrainModel(TrainModelReq& info)
{
    if (!PyFunLoad(exe_path_))
        return false;
    info_ = info;
    // 启动子线程前执行，释放PyEval_InitThreads获得的全局锁，否则子线程可能无法获取到全局锁。
    PyEval_ReleaseThread(PyThreadState_Get());

    // 1、训练、验证；2、更新算法对象
    std::thread train_thread(&CAlgHandle::Train, this);
    std::thread eval_thread(&CAlgHandle::Eval, this);

    train_thread.join();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (!train || !eval)
    {
        LOG_ERROR("train or eval failed");
        return false;
    }

    // 保证子线程调用都结束
    PyGILState_Ensure();

    PyObject* pRet = PyObject_CallFunctionObjArgs(py_stop_fun_, NULL);
    if (NULL == pRet)
    {
        LOG_ERROR("TrainModel failed");
        return false;
    }
    eval_thread.join();

    //char bufc[256] = { 0 };
    char *bufc;
    int a;
    float f;
    if (!PyArg_ParseTuple(pRet, "sif", &bufc,&a,&f))
    {
        LOG_ERROR("PyArg_ParseTuple failed");
        return false;
    }
    std::string model(bufc);
    LOG_INFO_FMT("train model [%s]", model.c_str());
    if (Test(model))
    {
        // 这里最新模型加载失败时，不回退
        std::string uff_model = g_config_.model_path + g_config_.uff_model_file;
        if (engine_.LoadUffAndSave(uff_model))
            return true;
    }

    return false;
}

void CAlgHandle::Discern(DiscernReq& req, DiscernRsp& rsp)
{
    // 图片加载
    std::vector<string> files;
    std::string pattern = "*";
    if(!recurse_get_file(files, req.img_path, pattern.c_str()))
    {
        LOG_WARN_FMT("get file failed. path[%s]", req.img_path.c_str());
        return;
    }
    int size = files.size();
    int max_size = engine_.GetMaxBatchSize();
    if (size > max_size)
    {
        LOG_WARN_FMT("batch[%d] max [%d]", size, max_size);
        return;
    }
    LOG_INFO_FMT("batch[%d] max [%d]", size, max_size);

    std::vector<cv::Mat> imgs;
    int i = 0;
    size_t pos = 0;
    std::string file_name;
    for (auto& it : files)
    {
        pos = it.find_last_of('/');
        file_name = it.substr(pos);
        LOG_INFO_FMT("idx[%d] img[%s]", i++, file_name.c_str());
        cv::Mat img = cv::imread(it);
        imgs.emplace_back(img);
    }

    // 识别
    if (engine_.Inference(imgs, rsp.label))
        LOG_INFO("inference success");
    else
        rsp.err_msg = "inference failed.";
}

bool CAlgHandle::PyFunLoad(std::string& exe_path)
{
    if (!Py_IsInitialized())
    {
        LOG_ERROR("Py_IsInitialized failed");
        return false;
    }

    LOG_INFO_FMT("exe path[%s]", exe_path.c_str());
    std::string cmd = "sys.path.append('/home/wangyinzhi/study/TensorFlow/cmake-build/mnist_LeNet5_CPPPY/bin/')";
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cmd.c_str());
    PyRun_SimpleString("print(sys.path)");
    PyRun_SimpleString("print(sys.version_info)");

    // 载入脚本
    static PyObject *train_model = PyImport_ImportModule("train");
    if (!train_model)
    {
        LOG_ERROR("can't find train.py");
        return false;
    }
    // 获取函数字典
    static PyObject *train_dict = PyModule_GetDict(train_model);
    if (!train_dict)
    {
        LOG_ERROR("PyModule_GetDict failed");
        return false;
    }
    // 找出函数名为train的函数
    py_train_fun_ = PyDict_GetItemString(train_dict, "train");
    if (!py_train_fun_ || !PyCallable_Check(py_train_fun_))
    {
        LOG_ERROR("can't find function [train]");
        return false;
    }

    // test函数
    static PyObject *test_model = PyImport_ImportModule("testing");
    if (!test_model)
    {
        LOG_ERROR("can't find testing.py");
        return false;
    }
    static PyObject *test_dict = PyModule_GetDict(test_model);
    if (!test_dict)
    {
        LOG_ERROR("PyModule_GetDict failed");
        return false;
    }
    py_test_fun_ = PyDict_GetItemString(test_dict, "testing");
    if (!py_test_fun_ || !PyCallable_Check(py_test_fun_))
    {
        LOG_ERROR("can't find function [testing]");
        return false;
    }

    // evaluate函数
    static PyObject *evaluate_model = PyImport_ImportModule("evaluate");
    if (!evaluate_model)
    {
        LOG_ERROR("can't find evaluate.py");
        return false;
    }
    static PyObject *evaluate_dict = PyModule_GetDict(evaluate_model);
    if (!evaluate_dict)
    {
        LOG_ERROR("PyModule_GetDict failed");
        return false;
    }
    py_eval_fun_ = PyDict_GetItemString(evaluate_dict, "evaluate");
    py_stop_fun_ = PyDict_GetItemString(evaluate_dict, "stop");
    if (!py_eval_fun_ || !PyCallable_Check(py_eval_fun_))
    {
        LOG_ERROR("can't find function [evaluate]");
        return false;
    }
    if (!py_stop_fun_ || !PyCallable_Check(py_stop_fun_))
    {
        LOG_ERROR("can't find function [stop]");
        return false;
    }
    return true;
}

void CAlgHandle::Train() 
{
    // 获取python环境全局锁
    CPyThreadStateLock pylock;
    train = true;
    // 设置参数
    std::string path = g_config_.model_path + g_config_.model_name;
    LOG_INFO_FMT("model file[%s] step[%d]", path.c_str(), info_.training_steps);
    PyObject* arg1 = PyBytes_FromString(path.c_str());
    PyObject* arg2 = Py_BuildValue("i", info_.training_steps);
    PyObject* arg3 = Py_BuildValue("f", info_.rate_base);
    PyObject* arg4 = Py_BuildValue("f", info_.rate_decay);
    // 调用函数
    PyObject* pRet = PyObject_CallFunctionObjArgs(py_train_fun_, arg1, arg2, arg3, arg4, NULL);
    if (NULL == pRet)
    {
        LOG_ERROR("py_train_fun_ failed");
        train = false;
    }
    Py_DECREF(arg1);
    Py_DECREF(arg2);
    Py_DECREF(arg3);
    Py_DECREF(arg4);
}

void CAlgHandle::Eval()
{
    // 获取python环境全局锁
    CPyThreadStateLock pylock;
    eval = true;
    LOG_INFO("eval start");
    // 设置参数
    PyObject* arg1 = PyBytes_FromString(g_config_.model_path.c_str());
    // 调用函数
    PyObject* pRet = PyObject_CallFunctionObjArgs(py_eval_fun_, arg1, NULL);
    if (NULL == pRet)
    {
        LOG_ERROR("py_eval_fun_ failed");
        eval = false;
    }
    Py_DECREF(arg1);
}

bool CAlgHandle::Test(std::string& modle)
{
    eval = true;
    std::string pd_model = g_config_.model_path + g_config_.pb_model_file;
    std::string uff_model = g_config_.model_path + g_config_.uff_model_file;
    LOG_INFO_FMT("pb model [%s]", pd_model.c_str());
    LOG_INFO_FMT("uff model [%s]", uff_model.c_str());
    // 设置参数
    PyObject* arg1 = PyBytes_FromString(modle.c_str());
    PyObject* arg2 = PyBytes_FromString(pd_model.c_str());
    PyObject* arg3 = PyBytes_FromString(uff_model.c_str());
    // 调用函数
    PyObject* pRet = PyObject_CallFunctionObjArgs(py_test_fun_, arg1, arg2, arg3, NULL);
    Py_DECREF(arg1);
    Py_DECREF(arg2);
    Py_DECREF(arg3);
    if (NULL == pRet)
    {
        LOG_ERROR("py_test_fun_ failed");
        eval = false;
        return false;
    }
    LOG_INFO("Test success");
    return true;
}
