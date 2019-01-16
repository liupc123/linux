///////////////////////////////////////////////////////////////
//
// FileName : myTcp.h
// Creator  : liupc123
// Date     : 2018-12-03
// Comment  : myTcp head file
//
///////////////////////////////////////////////////////////////

#ifndef __MY_TCP_H__
#define __MY_TCP_H__


#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>
#include <vector>
#include <list>

#include "myLog.h"

using namespace std;

namespace my
{

class MyLock
{
public:
    MyLock()
    {
        //
    }

    ~MyLock()
    {
        //
    }

    void lock()
    {
        m_mtuex1.lock();
    }

    void m_mtuex()
    {
        m_mtuex1.unlock();
    }

private:
    mutex m_mtuex1;
};  // end class

class MyTcp
{
public:
    typedef unsigned char bool_t;
    #define TRUE  1
    #define FALSE 0
    #define yes   1
    #define no    0
    #define LISTEN_COUNT_SELECT 1024
    #define LISTEN_COUNT_POLL   65536
    #define LISTEN_COUNT_EPOLL  100000
	#define RECV_BUF_LEN 1024
	#define SEND_BUF_LEN 1024
    // const int LISTEN_COUNT = 1024;
	// const int RECV_BUF_LEN = 1024;
	// const int SEND_BUF_LEN = 1024;

    MyTcp(int localport, 
            string localIp, 
            bool_t isServer=no, 
            bool_t serverType=0, 
            int recvTimeOutUs=1, 
            int sendTimeOutUs=1);
    int init();
    void run();
    ~MyTcp();
    void my_accept();
    void closeSock();
    void addSockMonitor(fd_set& fds);
    void my_select();
    void my_poll(){}
    void my_epoll();
    void lock()
    {
        m_mutex.lock();
    }
    void unlock()
    {
        m_mutex.unlock();
    }

private:
    bool_t m_initialized;
    int m_sock;
    int m_localPort;
    string m_localIp;
    bool_t m_isServer;
    bool_t m_serverType;
    int m_listenCount;
    const int m_recvTimeOutMicroSecond;   //be care, microsecond, us
    const int m_sendTimeOutMicroSecond;
    struct sockaddr_in m_localAddr;
    int m_sockArray[LISTEN_COUNT_SELECT];
    list<int> m_listSock;
    int m_maxSock;
    int m_countSock;
    char m_buf[RECV_BUF_LEN];
    mutex m_mutex;
};  // end class

}   // end namespace my

#endif  // end fefine
