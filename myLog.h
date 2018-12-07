#ifndef _MYLOG_H__
#define _MYLOG_H__

#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <stdarg.h>

using std::ofstream;
using std::string;
using std::queue;
using std::cout;
using std::endl;

/*
* @class MyLog
* @brief Basic logging class
* @author liupc123 (http://blog.sina.com.cn/liupc123)
* @version v_1.1
* @date 2013/09/08
* @copy-right forever
*/

const int iMyLogDataSize = 1024;

enum eMyLogType
{
    LOG_TYPE_FATAL,
    LOG_TYPE_ERROR,
    LOG_TYPE_WARN,
    LOG_TYPE_INFO,
    LOG_TYPE_DEBUG
};

enum eMyLogOutput
{
    LOG_OUTPUT_COUT = 1,
    LOG_OUTPUT_FILE = 2
};


class MyLog{
public:
    static MyLog* getIns();
    int init(const std::string& fileName, eMyLogOutput eOutput);
    int init(const std::string& strFileName, string strOutput);
    int init();
    int write(const char* format, ... );

private:
    bool m_bInitialised;
    eMyLogType m_eMyLogType;
    eMyLogOutput m_eMyLogOutput;
    string m_strFileName;
    queue<std::string> m_queueData;
    ofstream m_ofStream;
    static MyLog* m_instance;

    MyLog();
    MyLog(const MyLog& other) = delete;
    MyLog& operator=(const MyLog& other) = delete;
    ~MyLog();
};

string getStrTime();

#define LOG_INFO(fmt, ... )\
{\
	char buffer[iMyLogDataSize - 128];\
    string strTemp = getStrTime();\
	snprintf(buffer,sizeof(buffer),"[%s %s %d %s INFO]: %s\n", __FILE__, __FUNCTION__, __LINE__, strTemp.c_str(), fmt);\
	MyLog::getIns()->write(buffer, ##__VA_ARGS__);\
}
#define LOG_ERROR(fmt, ... )\
{\
    char buffer[iMyLogDataSize + 128];\
    string strTemp = getStrTime();\
    snprintf(buffer,sizeof(buffer),"[%s %s %d %s ERROR]: %s\n", __FILE__, __FUNCTION__, __LINE__, strTemp.c_str(), fmt);\
    MyLog::getIns()->write(buffer, ##__VA_ARGS__);\
}

#endif


