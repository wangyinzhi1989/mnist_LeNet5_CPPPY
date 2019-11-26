/**
*  @file     sysutils.cpp
*  @brief    工具类及函数实现文件
*
*  @author   wangyinzhi
*  @date     2017/11/23
*
*  Change History : \n
*   -wangyingzhi 创建 2017/11/23
*/

#include <assert.h>
#include <algorithm>
#include "sysutils.h"
#include <ctime>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#endif // WIN32

string get_cur_date(const int fm /*=1*/)
{
    if (0 >= fm){
        return "";
    }

    time_t iTime = time(0);
    struct tm* tmTime = localtime(&iTime);

    if (NULL == tmTime){
        return "";
    }

    string sfmt;
    switch (fm)
    {
    default:
    case 1:
        sfmt = "%04d-%02d-%02d"; break;
    }
    int nYear = 1900 + tmTime->tm_year;
    int nMon = 1 + tmTime->tm_mon;
    int nDay = tmTime->tm_mday;
    int nHour = tmTime->tm_hour;
    int nMin = tmTime->tm_min;
    int nSec = tmTime->tm_sec;

    char szCompleteTime[100] = { 0 };
    sprintf(szCompleteTime, sfmt.c_str(), nYear, nMon, nDay);
    std::string szLocalTime = szCompleteTime;

    return szLocalTime;
}

string get_cur_time(const int fm /*= 1*/, int addSec /*= 0*/)
{
    if (0 >= fm) {
        return "";
    }

    time_t iTime = time(0);
    iTime += addSec;
    struct tm* tmTime = localtime(&iTime);

    if (NULL == tmTime) {
        return "";
    }

    string sfmt;
    switch (fm)
    {
    default:
    case 1:
        sfmt = "%04d-%02d-%02d %02d:%02d:%02d"; break;
    case 2:
        sfmt = "%04d-%02d-%02dT%02d:%02d:%02d"; break;
    }
    int nYear = 1900 + tmTime->tm_year;
    int nMon = 1 + tmTime->tm_mon;
    int nDay = tmTime->tm_mday;
    int nHour = tmTime->tm_hour;
    int nMin = tmTime->tm_min;
    int nSec = tmTime->tm_sec;

    char szCompleteTime[100] = { 0 };
    sprintf(szCompleteTime, sfmt.c_str(), nYear, nMon, nDay, nHour, nMin, nSec);
    std::string szLocalTime = szCompleteTime;

    return szLocalTime;
}

#ifdef WIN32
///convert wide chars to multibytes
static char* wide_to_mtbytes(int c, const wchar_t* pWideText)
{
    assert(pWideText);

    int size = WideCharToMultiByte(c, 0, (LPCWSTR)pWideText, -1, NULL, 0, NULL, NULL);
    if (size == 0) {
        assert(false);
        return NULL;
    }
    char* pText = new char[size + 1];
    if (WideCharToMultiByte(c, 0, (LPCWSTR)pWideText, -1, pText, size, NULL, NULL) == 0) {
        delete[]pText;
        assert(false);
        return NULL;
    }
    pText[size] = '\0';
    return pText;
}

///convert multibytes to wide chars
static wchar_t* mtbytes_to_wide(int c, const char* pText)
{
    assert(pText);

    wchar_t* pWideText = NULL;
    int size = MultiByteToWideChar(c, 0, pText, -1, NULL, 0);
    if (size == 0) {
        assert(false);
        return pWideText;
    }
    else {
        pWideText = new wchar_t[size + 1];
        if (MultiByteToWideChar(c, 0, pText, -1, (LPWSTR)pWideText, size) == 0) {
            delete[]pWideText;
            pWideText = NULL;
            assert(false);
            return pWideText;
        }
        else {
            pWideText[size] = 0;
            return pWideText;
        }
    }
}
#endif // WIN32

wstring utf8_to_wide(const char* pText)
{
    assert(pText);
    wchar_t* pWide = NULL;
#ifdef WIN32
    pWide = mtbytes_to_wide(CP_UTF8, pText);
#endif // WIN32
    assert(pWide);
    wstring s = (wchar_t*)pWide;
    delete[]pWide;
    return s;
}

string utf8_to_locale(const char* pText)
{
    if (NULL == pText)
        return "";
    wstring ws = utf8_to_wide(pText);
    char* pANSI = NULL;
#ifdef WIN32
    pANSI = wide_to_mtbytes(CP_ACP, ws.c_str());
#endif // WIN32
    if (pANSI == NULL) {
        assert(false);
        return "";
    }

    string r = pANSI;
    delete[]pANSI;
    return r;
}

string locale_to_utf8(const char* pText)
{
    if (NULL == pText)
        return "";

    wchar_t* pWideText = NULL;
#ifdef WIN32
    pWideText = mtbytes_to_wide(CP_ACP, pText);
#endif // WIN32
    if (pWideText == NULL) {
        assert(false);
        return "";
    }
    char* pUTF8 = NULL;
#ifdef WIN32
    pUTF8 = wide_to_mtbytes(CP_UTF8, pWideText);
#endif // WIN32
    if (pUTF8 == NULL) {
        assert(false);
        delete[]pWideText;
        return "";
    }
    string r = pUTF8;
    delete[]pUTF8;
    delete[]pWideText;
    return r;
}

string include_path_separator(const char* path)
{
    char cSeparator = '/';
    assert(path);
    string r = path;
    string::size_type len = r.length();
    if (len > 0) {
        if (r[len-1] == '/' || r[len-1] == '\\') {
            r[len-1] = cSeparator;
        } else {
            r += cSeparator;
        }
    } else {
        r += cSeparator;
    }
    return r;
}

string extract_file_path(const string& path)
{
    string::size_type ps = string::npos;
    string::size_type ps1 = path.rfind('/');
    string::size_type ps2 = path.rfind('\\');

    if (ps1 != string::npos && ps2 != string::npos){
#ifdef WIN32
        ps = max(ps1,ps2);
#else
        ps = std::max(ps1, ps2);
#endif // WIN32
    }else if(ps1 == string::npos){
        ps = ps2;
    }else{
        ps = ps1;
    }

    if (ps == string::npos){
        return include_path_separator(path.c_str());
    }
    return path.substr(0,ps);
}

string expand_path(const char* path)
{
#ifdef WIN32
    DWORD nLen = GetFullPathNameA(path, 0, NULL, NULL);
    if (nLen == 0) {
        return "";
    }

    nLen = nLen * 2 + 2;
    char* pOutBuf = new char[nLen];
    char* pName;

    nLen = GetFullPathNameA(path, nLen / 2, pOutBuf, &pName);

    string sPath(pOutBuf);
    delete[] pOutBuf;
    return sPath;
#else
    return "";
#endif
}

string get_executable_path()
{
#ifdef WIN32
    char pBuf[MAX_PATH + 1] = { 0 };
    DWORD size = GetModuleFileNameA(NULL, pBuf, MAX_PATH);
    if (size == 0) {
        return "";
    }
    return expand_path(pBuf);
#else
    char buf[4096] = {0};
    ssize_t count = readlink( "/proc/self/exe", buf, 4096);
    if ( count < 0 || count >= 4096 ){
        return "";
    }
    return buf;
#endif // WIN32
}

string get_executable_dir()
{
    string sExePath = get_executable_path();
    if (sExePath.empty()){
        return "";
    }
    return include_path_separator(extract_file_path(sExePath).c_str());
}

bool file_exists(const string& file, int mode)
{
    if (file.empty())
        return false;
#ifdef WIN32
    return 0 == _access(file.c_str(), mode);
#else
        return false;
#endif // WIN32
}

HANDLE file_open(const string& fileName, int mode)
{
    if (0 > mode || mode > 2)
        return NULL;
#ifdef WIN32
    const unsigned int accessMode[3] = {
        GENERIC_READ,
        GENERIC_WRITE,
        GENERIC_READ | GENERIC_WRITE
    };

    const unsigned int ShareMode[3] = {
        FILE_SHARE_READ,
        FILE_SHARE_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE
    };
    return CreateFileA(fileName.c_str(), accessMode[mode], ShareMode[mode], NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
        return 0;
#endif // WIN32
}

HANDLE file_create(const string& fileName)
{
    if (fileName.empty())
        return NULL;
#ifdef WIN32
    return CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
        return 0;
#endif // WIN32
}

void file_close(HANDLE handle)
{
    if (NULL == handle)
        return;
#ifdef WIN32
    CloseHandle(handle);
#else

#endif // WIN32
}

pair<bool, unsigned long> file_read(HANDLE handle, void * buffer, unsigned long nCount /*= ULONG_MAX*/)
{
    if (nCount == 0)
        return pair<bool, unsigned long>(true, 0);

    unsigned long nReadCount = 0;
#ifdef WIN32
    if (0 == ReadFile(HANDLE(handle), buffer, nCount, LPDWORD(&nReadCount), NULL))
        return pair<bool, unsigned long>(false, 0);

    return pair<bool, unsigned long>(true, nReadCount);
#else
    return pair<bool, unsigned long>(false, nReadCount);
#endif // WIN32
}

pair<bool, unsigned long> file_write(HANDLE handle, const void * buffer, unsigned long nCount)
{
    if (nCount == 0)
        return pair<bool, unsigned long>(true, 0);

    unsigned long nWriteCount = 0;
#ifdef WIN32
    if (0 == WriteFile(HANDLE(handle), buffer, nCount, LPDWORD(&nWriteCount), NULL))
        return pair<bool, unsigned long>(false, 0);

    return pair<bool, unsigned long>(true, nWriteCount);
#else
    return pair<bool, unsigned long>(false, nWriteCount);
#endif // WIN32
}

long long file_get_size(const string& fileName)
{
#ifdef WIN32
    WIN32_FILE_ATTRIBUTE_DATA iData;
    if (FALSE == GetFileAttributesExA(fileName.c_str(), GetFileExInfoStandard, &iData))
        return -1;
    long long nSize = iData.nFileSizeHigh;
    nSize = nSize << 32;
    nSize += iData.nFileSizeLow;
    return nSize;
#else
    return 0;
#endif // WIN32
}

bool recurse_get_file(vector<string>& vFiles, const string& sDirPath, const char* pattern)
{
    DIR *dir;
    struct dirent *ptr;

    if ((dir = opendir(sDirPath.c_str())) == NULL)
        return false;

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
        struct stat st;
        const std::string path{sDirPath + "/" + ptr->d_name};
        lstat(path.c_str(), &st);
        if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode))  // file
        {
            string fileName = include_path_separator(sDirPath.c_str()) + "/" + ptr->d_name;
            if (patternMatch(ptr->d_name, pattern, true))
                vFiles.push_back(std::move(fileName));
        } else if (S_ISDIR(st.st_mode))  //dir
        {
            string dirName = include_path_separator(sDirPath.c_str()) + "/" + ptr->d_name;
            recurse_get_file(vFiles, dirName, pattern);
        }
    }
    closedir(dir);
    return true;
}

bool patternMatch(const char* text, const char* pattern, bool bCaseSens /*= false*/)
{
    switch (pattern[0])
    {
    case '\0':
        return !text[0];
    case '*':
        return patternMatch(text, pattern + 1, bCaseSens) || (text[0] && patternMatch(text + 1, pattern, bCaseSens));
    case '?':
        return text[0] && patternMatch(text + 1, pattern + 1, bCaseSens);
    default:
        if (!bCaseSens)
            return (toupper(pattern[0]) == toupper(text[0])) && patternMatch(text + 1, pattern + 1, bCaseSens);
        else
            return (pattern[0] == text[0]) && patternMatch(text + 1, pattern + 1, bCaseSens);
    }
}

void trim(string &s, const char regex /*= ' '*/)
{
    if (s.empty())
        return;

    s.erase(0, s.find_first_not_of(regex));
    s.erase(s.find_last_not_of(regex) + 1);
}

void split(std::vector<string> &result, const string &input, const char regex)
{
    int pos = 0;
    int npos = 0;
    while ((npos = input.find(regex, pos)) != -1)
    {
        string tmp = input.substr(pos, npos - pos);
        result.push_back(std::move(tmp));
        pos = npos + 1;
    }
    result.push_back(std::move(input.substr(pos, input.length() - pos)));
}

bool str_to_hex(const string & src, OUT char* dst, int len /*=0*/)
{
    if (src.empty())
        return true;

    if (0 == len)
        len = src.length();

    const char *cSrc = src.c_str();
    BYTE high, low;
    int idx, ii = 0;
    for (idx = 0; idx < len; idx += 2)
    {
        high = cSrc[idx];
        low = cSrc[idx + 1];

        if (high >= '0' && high <= '9')
            high = high - '0';
        else if (high >= 'A' && high <= 'F')
            high = high - 'A' + 10;
        else if (high >= 'a' && high <= 'f')
            high = high - 'a' + 10;
        else
            return false;

        if (low >= '0' && low <= '9')
            low = low - '0';
        else if (low >= 'A' && low <= 'F')
            low = low - 'A' + 10;
        else if (low >= 'a' && low <= 'f')
            low = low - 'a' + 10;
        else
            return false;

        dst[ii++] = high << 4 | low;
    }
    return true;
}

void get_local_ips(std::vector<std::string>& ips)
{
#ifdef WIN32
    return;
#else
    struct ifaddrs *ifAddrStruct = NULL;
    void *tmpAddrPtr = NULL;
    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL) {
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {
            tmpAddrPtr = &((struct sockaddr_in *) ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            if (strncmp(addressBuffer, "127.0.0.1", strlen("127.0.0.1")) != 0) {
                ips.push_back(std::string(addressBuffer));
            }
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }

#endif //WIN32
}

