/**
*  @file     py_thread_state_lock.h
*  @brief    python线程互斥锁类定义文件.
*
*  @author   wangyinzhi
*  @date     2019/08/26
*
*  Change History : \n
*   -wangyingzhi 创建 2019/08/26
*/

#ifndef TN_TSE_PY_THREAD_STATE_LOCK_H
#define TN_TSE_PY_THREAD_STATE_LOCK_H

#include "Python.h"

/**
* @brief CPyThreadStateLock类
* python线程互斥锁
**/
class CPyThreadStateLock
{
public:
    CPyThreadStateLock(void)
    {
        state = PyGILState_Ensure();
    }

    ~CPyThreadStateLock(void)
    {
        PyGILState_Release(state);
    }
private:
    PyGILState_STATE state;
};

#endif // TN_TSE_PY_THREAD_STATE_LOCK_H
