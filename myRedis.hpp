#ifndef __MY_REDIS_H__
#define __MY_REDIS_H__

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>
#include <hiredis.h>

#include "myLog.h"

using namespace std;

namespace my
{

class MyRedis
{
public:
    typedef char bool_t;
    #define YES    1;
    #define NO     0;
    #define TRUE   1;
    #define FALSE  0;
    #define ERROR  -1;

    MyRedis(int iPort=6379, string strIp="127.0.0.1", int iIndexDB=0)
    {
        m_initialize = FALSE;
        m_serverPort = iPort;
        m_serverIp = strIp;
        m_conn = NULL;
        m_reply = NULL;
        init(iIndexDB);
    }

    int init(int iIndexDB)
    {
        if ( m_initialize )
        {
            return -1;
        }

        struct timeval timeout = { 2, 0 }; // 2 seconds
        m_conn = redisConnectWithTimeout(m_serverIp.c_str(), m_serverPort, timeout);
        if (m_conn == NULL || m_conn->err)
        {
            if (m_conn)
            {
                LOG_ERROR("Connection error: %s", m_conn->errstr);
                close();
            }
            else
            {
                LOG_ERROR("Connection error: can't allocate redis context");
            }
            return -1;
        }

        m_initialize = TRUE;

        char ch[10];
        bzero(ch, sizeof(ch));
        sprintf(ch, "select %d", iIndexDB);
        int ret = 0;
        string str = "";
        exec_cmd(ch, ret, str);

        return 0;
    }

    inline void exec_cmd(string strCmd, int& iRet, string& strRet)
    {
        free();
        if ( m_initialize )
        {
            m_reply = (redisReply*)redisCommand(m_conn, strCmd.c_str());
            if ( m_reply )
            {
                // LOG_INFO("%s: %d, %s", strCmd.c_str(), m_reply->integer, m_reply->str);
                iRet = m_reply->integer;
                if ( m_reply->str )
                {
                    strRet = m_reply->str;
                }
                displayReplay();
            }
            else
            {
                LOG_ERROR("excute cmd %s error", strCmd.c_str());
            }
        }
        else
        {
            LOG_ERROR("unconnected");
        }
    }

    ~MyRedis()
    {
        close();
    }


private:
    inline void displayReplay()
    {
        if ( !m_reply )
        {
            return;
        }
        
        stringstream ss;
        ss << endl;
        // ss << "type: " << m_reply->type << "\n";
        if ( m_reply->integer )
        {
            ss << "integer: " << m_reply->integer << "\n";
        }
        if ( m_reply->len )
        {
            // ss << "len: " << m_reply->len << "\n";
            ss << "str: " << m_reply->str << "\n";
        }
        redisReply* element = NULL;
        for ( int i=0; i<m_reply->elements; ++i)
        {
            element = m_reply->element[i];
            // ss << "type: " << element->type << "\n";
            if ( m_reply->integer )
            {
                ss << "integer: " << m_reply->integer << "\n";
            }
            if ( element->len )
            {
                // ss << "len: " << element->len << "\n";
                ss << "str: " << element->str << "\n";
            }
        }
        LOG_INFO("%s", ss.str().c_str() );
    }

    inline void free()
    {
        freeReplyObject(m_reply);
    }

    void close()
    {
        if ( m_conn )
        {
            redisFree(m_conn);
            m_conn = NULL;
        }
        m_initialize = FALSE;
    }

    bool_t m_initialize;
    string m_serverIp;
    int m_serverPort;
    redisContext* m_conn;
    redisReply* m_reply;
};

} //end namespace

#endif  //__MY_REDIS_H__
