#ifndef _MY_FTP_CLIENT_
#define _MY_FTP_CLIENT_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#include "myLog.h"
 
using namespace std;
 
//数据传输模式选择
#define PORT 1
#define PASV 2
#define BUFSIZE 1024
#define DATABUFSIZE 1024
#define ROOTDIRLENGTH 128
#define FILENAMESIZE 12

class MyFtpClient 
{
private:
	int m_iCmdSock;					//控制socket
	int m_iDataSock;				//数据socket
	string m_strServerIp;			//服务器IP
	string m_strServerPort;			//服务器Port
	string m_strUser;				//服务器user
	string m_strPwd;				//服务器pwd
	string m_strCurTransferMode;	//当前的数据传输模式，PORT或PASV
	char m_buffer[BUFSIZE];			//控制连接所使用的缓存
	char m_dataBuffer[DATABUFSIZE];	//数据连接所使用的缓存

	string m_strCurrentPath;
	int m_iFileCount;
	int m_iInitialized;
	int m_login;

	// int fileNo;
	// string curdir;
	// char buf[BUFSIZE];
	// char dataBuf[DATABUFSIZE];
	// string m_strCurentDir;	//当前路径,结尾不应当有'\n'，另外，最后一个字符可能是'/'也可能不是
	// char comtradeFileName[64][13];

public:
	MyFtpClient(const string& ftpServerIp, const string& ftpServerPort, const string& ftp_user, const string& ftp_pwd);
	~MyFtpClient();
	MyFtpClient(const MyFtpClient& other) = delete;
	MyFtpClient& operator=(const MyFtpClient& other) = delete;

	int connectToServer();//该函数是用来连接服务器的

	int sendToServer(const string& cmd, const string& msg);		//send cmd, return the return value of ::send
	int recvFromServer();	//recv msg after send cmd	return the length of recv, others, return -1;
	
	int login();//该函数是用来登陆服务器的
	
	void logout();//该函数是用来退出服务器的
	
	int createDataSocket();	//该函数主要是用来建立数据连接的
	
	void closeDataSocket();	//关闭数据连接
	
	void get(const string& serverFile, const string& localFile);	//上传文件//下载文件
	
	int put(const string& localFile, const string& serverFile);	//上传文件
	int put(const string& localFile);

	bool cd(const string& dstPath);	//该函数式用来切换目录

	string pwd();

//	string getDestdir(char* word);//将输入指令中的路径转化为服务器绝对路径
	
//	void dir(string destdir);	//该函数是用来查看当前目录下的文件信息
	
//	void analysisFileName(char *data);	//该函数解析文件名
	
//	void ascii();
 
//	bool main_open(char * address, char* user, char* pass, const char* port="21");
//	void main_get(char * serverfile, char * localfile);
};

#endif