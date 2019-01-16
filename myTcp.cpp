///////////////////////////////////////////////////////////////
//
// FileName : myTcp.h
// Creator  : liupc123
// Date     : 2018-12-03
// Comment  : myTcp source file
//
///////////////////////////////////////////////////////////////

#include "myTcp.h"

namespace my
{

MyTcp::MyTcp(int localPort,
            string localIp,
            bool_t isServer,
            bool_t serverType,
            int recvTimeOutUs,
            int sendTimeOutUs) :
    m_localPort(localPort),
    m_localIp(localIp),
    m_isServer(isServer),
    m_serverType(serverType),
    m_recvTimeOutMicroSecond(recvTimeOutUs),
    m_sendTimeOutMicroSecond(sendTimeOutUs)
{
    m_countSock = 0;
    m_listSock.clear();
    m_sock = -1;
    m_initialized = FALSE;
    m_listenCount = LISTEN_COUNT_SELECT;
    bzero(m_sockArray, LISTEN_COUNT_SELECT);
    bzero(&m_localAddr, sizeof(m_localAddr) );

    if ( 0 == init() )
    {
        LOG_INFO("create MyTcp ok, sock: %d localIp-port: %s-%d", m_sock, m_localIp.c_str(), m_localPort);
    }
    else
    {
        LOG_ERROR("create MyTcp error, localIp-port: %s-%d", m_localIp.c_str(), m_localPort);
    }
}

void analyse(char buf[], int len)
{
    if ( len > 0 )
    {
        LOG_INFO("read %d data: %s", len, buf);
    }
}

void MyTcp::my_accept()
{
    while ( yes )
    {
        struct sockaddr_in remoteAddr;
        bzero(&remoteAddr, sizeof(remoteAddr));
        socklen_t sock_len = sizeof(remoteAddr);
        int sockClient = accept(m_sock, (struct sockaddr*)&remoteAddr, &sock_len );
        if ( sockClient > 0 )
        {
            lock();
            m_listSock.push_back(sockClient);
            unlock();
        }
    }
}

int MyTcp::init()
{
    int ret = 0;
    if ( m_initialized )
    {
        LOG_ERROR("has been initialized, return");
        return m_initialized;
    }

    m_sock = socket(PF_INET, SOCK_STREAM, 0);
    if ( m_sock < 0 )
    {
        LOG_ERROR("create socket failed");
        return -1;
    }

    m_maxSock = m_sock;

    if ( m_localPort > 0 )  //>0:server, other: client
    {
        m_localAddr.sin_family = AF_INET;
        if ( m_localIp.empty() )
        {
            m_localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            m_localAddr.sin_addr.s_addr = inet_addr( m_localIp.c_str() );
        }
        m_localAddr.sin_port = htons(m_localPort);

        ret = bind(m_sock, (sockaddr*)&m_localAddr, sizeof(m_localAddr));
        if ( 0 != ret )
        {
            LOG_ERROR("bind error, ret: %d", ret);
            return ret;
        }
    }

    struct timeval timeOut{0, 0};
    if ( m_sendTimeOutMicroSecond > 0 )
    {
        if ( m_sendTimeOutMicroSecond < 100 )
        {
		    timeOut.tv_sec = m_sendTimeOutMicroSecond;
        }
        else
        {
            timeOut.tv_usec = m_sendTimeOutMicroSecond;
        }

		ret = setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, &timeOut, sizeof(timeOut));    // 接收时限
		if ( ret < 0 )
		{
			LOG_ERROR("setsockopt SendTimeOut failed, ret: %d, errno: %d", ret, errno);
		}
    }

    bzero(&timeOut, sizeof(timeOut) );
    if ( m_recvTimeOutMicroSecond > 0 )
    {
        if ( m_recvTimeOutMicroSecond < 100 )
        {
		    timeOut.tv_sec = m_recvTimeOutMicroSecond;
        }
        else
        {
            timeOut.tv_usec = m_recvTimeOutMicroSecond;
        }

		ret = setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut));    // 接收时限
		if ( ret < 0 )
		{
			LOG_ERROR("setsockopt RecvTimeOut failed, ret: %d, errno: %d", ret, errno);
		}
    }

    m_initialized = TRUE;
    return 0;
}

void MyTcp::addSockMonitor(fd_set& fds)
{
    if ( m_sock )
    {
        m_countSock = 0;
        bzero(m_sockArray, LISTEN_COUNT_SELECT);
        m_maxSock = m_sock;
        m_sockArray[m_countSock++] = m_sock;
        FD_ZERO(&fds);
        FD_SET(m_sock, &fds);
        for (auto& val : m_listSock)
        {
            m_sockArray[m_countSock++] = val;
            FD_SET(val, &fds);
            if ( val > m_maxSock )
            {
                m_maxSock = val;
            }
        }
    }
}

void MyTcp::run()
{

    if ( !m_initialized )
    {
        return ;
    }

    if ( m_isServer )
    {
        int ret = listen(m_sock, m_listenCount);
        if ( -1 == ret )
        {
            LOG_ERROR("listen failed, ret: %d, errno: %d", ret, errno);
            return;
        }

        switch( m_serverType )
        {
            case 0:
                break;

            case 1:
                my_select();
                break;

            case 2:
                my_poll();
                break;

            case 3:
                my_epoll();
                break;

            default:
                break;
        }//end switch

    }//end is server
}

void MyTcp::my_select()
{
    LOG_INFO("server model: select")
    fd_set rdfds; // struct
    fd_set exfds;
    struct timeval timeout = {1, 0};
    char buf[RECV_BUF_LEN];
    int readLen = 0;
    while ( yes )
    {
        addSockMonitor(rdfds);
        int ret = select(m_maxSock+1, &rdfds, NULL, &exfds, &timeout);
        if ( -1 == ret )
        {
            LOG_ERROR("select error, ret : %d, errno: %d", ret, errno);
            break;
        }
        else if ( 0 == ret )
        {
            continue;
        }
        else
        {
            for (int i=0; i<m_maxSock+1; ++i)
            {
                if( m_sockArray[i]>0 && FD_ISSET(m_sockArray[i], &rdfds) )
                {
                    if ( m_sockArray[i] == m_sock )
                    {
                        struct sockaddr_in remoteAddr;
                        bzero(&remoteAddr, sizeof(remoteAddr));
                        socklen_t sock_len = sizeof(remoteAddr);
                        int sockClient = accept(m_sock, (struct sockaddr*)&remoteAddr, &sock_len );
                        if ( sockClient > 0 )
                        {
                            m_listSock.push_back(sockClient);
                            m_sockArray[m_countSock++] = sockClient;
                            if ( sockClient > m_maxSock )
                            {
                                m_maxSock = sockClient;
                            }
                            LOG_INFO("accept new connect %d: %s-%d", sockClient, inet_ntoa(remoteAddr.sin_addr), remoteAddr.sin_port);
                        }
                    }
                    else
                    {
                        int repeat_num = 5;
                    repeat_read:
                        bzero(buf, RECV_BUF_LEN);
                        readLen = read(m_sockArray[i], buf, RECV_BUF_LEN);
                        if ( readLen > 0 )
                        {
                            analyse(buf, readLen);
                        }
                        else if ( readLen == 0 )
                        {
                            close(m_sockArray[i]);
                            m_listSock.remove(m_sockArray[i]);
                            LOG_INFO("remove sock-%d from listen", m_sockArray[i]);
                            FD_CLR(m_sockArray[i], &rdfds);
                            m_sockArray[i] = -1;
                        }
                        else
                        {
                            if ( errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN )
                            {
                                if ( repeat_num-- )
                                {
                                    usleep(10000);
                                    goto repeat_read;
                                    // bzero(buf, RECV_BUF_LEN);
                                    // readLen = read(m_sockArray[i], buf, RECV_BUF_LEN);
                                    // analyse(buf, readLen);
                                }
                                LOG_ERROR("read data error, ret: %d, errno: %d", readLen, errno);
                            }
                        }
                    }// end accept or read
                }// end FD_ISSET
            }// end for check sock array
        }// end select return
    }// end while
}

void MyTcp::my_epoll()
{
    LOG_INFO("server model: epoll")

    //1.创建epoll
    int epollFd = epoll_create(LISTEN_COUNT_EPOLL);
    if ( epollFd <= 0 )
    {
        LOG_INFO("epoll_create(%d) failed, ret: %d", LISTEN_COUNT_EPOLL, epollFd);
        return ;
    }
    
    //2.托管
    struct epoll_event epollEvent;  //epoll结构填充
    epollEvent.events = EPOLLIN;    //初始关心事件为读( |水平触发(Level-triggered),缺省模式 )
    epollEvent.data.fd = m_sock;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, m_sock, &epollEvent);  //将listen sock添加到epoll_fd中，关心读事件
    for ( auto& val: m_listSock )
    {
        epollEvent.events = EPOLLIN | EPOLLET;    //初始关心事件为读
        epollEvent.data.fd = val;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, m_sock, &epollEvent);
    }
    struct epoll_event revs[LISTEN_COUNT_EPOLL];
    int timeOutMicroSeconds = -1;
    while ( yes )
    {
        int num = epoll_wait(epollFd, revs, LISTEN_COUNT_EPOLL, timeOutMicroSeconds); //block until data readed
        switch ( num )  //返回需要处理的事件数目  LISTEN_COUNT_EPOLL表示 事件有多大
        {
            case 0: //返回0，表示监听超时
                LOG_INFO("epoll_wait timeout");
                break;

            case -1:    //出错
                LOG_INFO("epoll_wait error");
                break;

            default://大于零 即就是返回了需要处理事件的数目
            {
                char buf[1024];
                ssize_t readLen;
                struct sockaddr_in remoteAddr;
                bzero(&remoteAddr, sizeof(remoteAddr));
                socklen_t len = sizeof(remoteAddr);
                for(int i=0; i<num; i++)
                {
                    int sock = revs[i].data.fd; //准确获取哪个事件的描述符
                    if( sock == m_sock && revs[i].events && EPOLLIN ) //如果是初始的 就接受，建立链接
                    {
                        int sockClient = accept(m_sock, (struct sockaddr*)&remoteAddr, &len);
                        if( sockClient > 0 )
                        {
                            LOG_INFO("accept new connect %d: %s-%d", sockClient, inet_ntoa(remoteAddr.sin_addr), remoteAddr.sin_port);
                            // int fl = fcntl(sockClient, F_GETFL);
                            fcntl(sockClient, F_SETFL, O_NONBLOCK);
                            epollEvent.events = EPOLLIN | EPOLLET;
                            epollEvent.data.fd = sockClient;
                            m_listSock.push_back(sockClient);
                            epoll_ctl(epollFd, EPOLL_CTL_ADD, sockClient, &epollEvent); //二次托管
                        }
                    }
                    else // 接下来对 num-1 个事件处理
                    {
                        if( revs[i].events )//&& EPOLLIN )
                        {
                            int repeat_num = 5;
                        repeat_read:
                            bzero(buf, RECV_BUF_LEN);
                            readLen = read(sock, buf, sizeof(buf));
                            if( readLen > 0 )
                            {
                                analyse(buf, readLen);
                                // epollEvent.events = EPOLLIN | EPOLLET;
                                // epoll_ctl(epollFd, EPOLL_CTL_DEL, sock, &epollEvent); //二次托管
                            }
                            else if ( readLen == 0 )
                            {
                                close( sock );
                                m_listSock.remove(sock);
                                epoll_ctl(epollFd, EPOLL_CTL_DEL, sock, NULL);
                                LOG_INFO("remove sock-%d from listen", sock);
                            }
                            else
                            {
                                if ( errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN )
                                {
                                    if ( repeat_num-- )
                                    {
                                        usleep(10000);
                                        goto repeat_read;
                                    }
                                    // bzero(buf, RECV_BUF_LEN);
                                    // readLen = read(sock, buf, RECV_BUF_LEN);
                                    // analyse(buf, readLen);
                                }
                                LOG_ERROR("read data error, ret: %d, errno: %d", readLen, errno);
                            }
                        }
                    }
                }
                break;
            }// end default deal
        }// end switch
    }// end while
}

void MyTcp::closeSock()
{
    if ( m_sock > 0)
    {
        close(m_sock);
        m_sock = -1;
    }
}

MyTcp::~MyTcp()
{
    closeSock();
    m_listSock.clear();
}

}   //end namespace
