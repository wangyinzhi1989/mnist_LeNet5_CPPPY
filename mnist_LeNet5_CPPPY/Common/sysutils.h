/**
*  @file     sysutils.h
*  @brief    工具类及函数定义文件
*
*   部分支持linux
*
*  @author   wangyinzhi
*  @date     2017/11/23
*
*  Change History : \n
*   -wangyingzhi 创建 2017/11/23
*/

#ifndef __SYSUTILS_H__
#define __SYSUTILS_H__

#include <string>
#include <vector>
#include <climits>
#include <mutex>
#include "string.h"
#ifdef WIN32
#include <windows.h>
#else
typedef void *HANDLE;
typedef unsigned char BYTE;

#ifndef OUT
#define OUT
#endif // OUT

#ifndef ULONG_MAX
#define ULONG_MAX     0xffffffffUL
#endif // ULONG_MAX

#endif // WIN32

using namespace std;

/*********************************************  时间相关  *********************************************/
/*!
@brief 获得当前日期字符串
@param fm 格式化字符串 1:yyyy-mm-dd
@return 格式化后的字符串
*/
string get_cur_date(const int fm = 1);

/*!
@brief 获得当前时间字符串
@param fm 格式化字符串 1:yyyy-mm-dd hh:MM:ss 2:yyyy-mm-ddThh:MM:ss
@param addYear 当期日期的基础上增加的秒数
@return 格式化后的字符串
*/
string get_cur_time(const int fm = 1, int addSec = 0);

/*********************************************  编码转换相关  *********************************************/
/*!
@brief 将UTF8字符串转换成UNICODE字符串
@param pText UTF8字符串
@return UNICODE字符串
*/
wstring utf8_to_wide(const char* pText);

/*!
@brief 将UTF8字符串转换成本地字符串
@param pText UTF8字符串
@return 本地字符串
*/
string utf8_to_locale(const char* pText);

/*!
@brief 本地字符集至UTF8
@param pText 本地字符串
@return UTF8字符串
*/
string locale_to_utf8(const char* pText);

/*********************************************  可执行文件操作相关  *********************************************/
/*!
*@brief 给文件路径追加分隔符
*@note 最后一位为分隔符时，不会追加
*/
string include_path_separator(const char* path);

/*!
*@brief 获取文件路径
*/
string extract_file_path(const string& path);

/*!
*@brief 获取进程中可执行文件的路径
*@return 失败返回空字符串
*/
string get_executable_path();

/*!
*@brief 获取进程中可执行文件所在的目录
*@return 失败返回空字符串
*/
string get_executable_dir();

/*********************************************  文件操作相关  *********************************************/
// TODO 可使用boost库隔离底层
/*!
*@brief 文件是否存在
*@param  file  修剪对象
*@param  mode  权限标志。0-存在即可；2-写权限；4-读权限；6-读写权限
*@return true-存在；false-失败
*/
bool file_exists(const string& file, int mode = 0);

/*!
*@brief 打开文件
*@param fileName 含路径的文件名
*@param mode 打开文件的模式。0-读权限；1-写权限；2-读写权限
*@return 成功返回文件句柄，失败返回NULL
*/
HANDLE file_open(const string& fileName, int mode);

/*!
*@brief 创建文件
*@param fileName 含路径的文件名
*@return 成功返回文件句柄，失败返回NULL
*/
HANDLE file_create(const string& fileName);

/*!
*@brief 关闭文件
*@param handle 文件句柄
*@return void
*/
void file_close(HANDLE handle);

/*!
*@brief 从一个文件里读取Count字节到Buffer里
*@param handle 文件句柄
*@param buffer 缓冲区指针
*@param Count 要读取的字节数，默认读取全部
*@return first：true-成功，false-失败。second：first为true有效，表示读取到的字节数
*@note 这里的全部是系统能表示的unsigned long最大值
*/
pair<bool, unsigned long> file_read(HANDLE handle, void * buffer, unsigned long nCount = ULONG_MAX);

/*!
*@brief 将Buffer写入Count字节到一个文件里
*@param handle 文件句柄
*@param buffer 缓冲区指针
*@param Count 要写入的字节数
*@return first：true-成功，false-失败。second：first为true有效,表示写入的字节数
*/
pair<bool, unsigned long>  file_write(HANDLE handle, const void * buffer, unsigned long nCount);

/*
@brief 获得文件大小，单位字节
@param fileName 文件名
@return 失败返回-1
*/
long long file_get_size(const string& fileName);

/*!
@brief 递归查找文件
@param vFiles 查找结果，全路径列表,不包括目录
@param sDirPath 查找目录
@param pattern 查找匹配的名字, 使用*?进行匹配
@return true-成功，false-失败
*/
bool recurse_get_file(vector<string>& vFiles, const string& sDirPath, const char* pattern);

/*********************************************  字符操作相关  *********************************************/
/*!
@brief text 是否匹配pattern
@param text 等待匹配的字符串
@param pattern 模式字符串,其中*表示任意多个字符,?表示一个字符,其它字符表示字符本身
@param bCaseSens 是否大小写敏感
@return 是否成功匹配
*/
bool patternMatch(const char* text, const char* pattern, bool bCaseSens = false);

/*!
*@brief 修剪字符串，去除左右空格
*@param  s 待裁剪字符串
*@param  regex  修剪对象
*@return void
*@note 修剪后的字符串仍保存到s中
*/
void trim(string &s, const char regex = ' ');

/**
*@brief 拆分字符串
*@param  result 结果保存容器
*@param  input  待拆分字符串
*@param  regex  拆分标注
*@return true-成功；false-失败
*/
void split(std::vector<string> &result, const string &input, const char regex);


/**
*@brief 将字符串转换成字符数组
*@param src 待转目标
*@param dst 转换后结果容器
*@param dst 长度,为0时，转换全部
*@return true-成功；false-失败
*/
bool str_to_hex(const string & src, OUT char* dst, int len = 0);


/*********************************************  网络相关  *********************************************/
/**
*@brief 本地IP获取
*@param void
*@return ip容器
*/
void get_local_ips(std::vector<std::string>& ips);

/*********************************************  业务ID相关  *********************************************/
//! 任务id分发器
struct Id_Creater {
    unsigned int id_{ 0 };             //!< id
    std::mutex id_mutex_;         //!< 互斥量

    std::string operator()(const std::string& prefix)
    {
        id_mutex_.lock();
        unsigned int id = ++id_;
        // 这里去除最后10个值，以免溢出
        if (UINT_MAX - 10 == id_)
            id_ = 0;
        id_mutex_.unlock();
        return prefix + std::to_string(id);
    }
};

#endif // __SYSUTILS_H__