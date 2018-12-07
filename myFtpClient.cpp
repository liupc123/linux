#include "myFtpClient.h"
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>

MyFtpClient::MyFtpClient(const string& ftpServerIp, const string& ftpServerPort, const string& ftp_user, const string& ftp_pwd)
{
	m_strServerIp = ftpServerIp;
	m_strServerPort = ftpServerPort;
	m_strUser = ftp_user;
	m_strPwd = ftp_pwd;
	m_strCurTransferMode = "PASV";
	m_strCurrentPath = "/";
	m_iFileCount = 0;
	m_iInitialized = 0;
	m_login = 0;
	connectToServer();
}

MyFtpClient::~MyFtpClient()
{
	m_iFileCount = 0;
	m_iInitialized = 0;
	m_login = 0;
	close(m_iCmdSock);
	close(m_iDataSock);
	m_iDataSock = -1;
	m_iCmdSock = -1;
}

int MyFtpClient::connectToServer()
{
	if ( 0 < m_iInitialized )
	{
		LOG_ERROR("ftp has been initialized!");
		return -1;
	}

	m_iCmdSock = socket(AF_INET, SOCK_STREAM, 0);
	if ( m_iCmdSock < 0)
	{
		LOG_ERROR("ftp create socket error");
		return -1;
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_aton(m_strServerIp.c_str(), &addr.sin_addr);
	addr.sin_port = htons( atoi(m_strServerPort.c_str() ) );

    struct timeval timeOut = {15, 0};
    int ret = setsockopt(m_iCmdSock, SOL_SOCKET, SO_SNDTIMEO, &timeOut, sizeof(timeOut));    // send时限
    if ( ret < 0 )
    {
        close( m_iCmdSock );
        LOG_ERROR("setsockopt SendTimeOut failed: %d!", errno);
        return ret;
    }
	
    ret = setsockopt(m_iCmdSock, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut));    // recv时限
    if ( ret < 0 )
    {
        close( m_iCmdSock );
        LOG_ERROR("setsockopt RecvTimeOut failed: %d!", errno);
        return ret;
    }
	
	ret = connect(m_iCmdSock, (struct sockaddr*)&addr, sizeof(addr) );
	if ( ret < 0 )
	{
		LOG_ERROR("cmd socket connect to ftp server error, ip-port: %s-%s", m_strServerIp.c_str(), m_strServerPort.c_str() );
		return ret;
	}

	m_iInitialized = 1;
	LOG_INFO("cmd socket connect to ftp server success, ip-port: %s-%s", m_strServerIp.c_str(), m_strServerPort.c_str() );
	return 0;
}

int MyFtpClient::sendToServer(const string& cmd, const string& msg)
{
	string sendMsg = cmd + " " + msg + "\r\n";
	int ret = send(m_iCmdSock, sendMsg.c_str(), sendMsg.length(), 0);
	LOG_INFO("ret: %d, send to ftp server: %s %s", ret, cmd.c_str(), msg.c_str());
	return ret;
}

int MyFtpClient::recvFromServer()
{
	bzero(m_buffer, BUFSIZE);
	int ret = recv(m_iCmdSock, m_buffer, BUFSIZE, 0);
	LOG_INFO("ret: %d, recv from ftp server: %s", ret, m_buffer);
	return ret;
}

/*
响应码用三位数字编码表示：
第一个数字给出了命令状态的一般性指示，比如响应成功、失败或不完整。
第二个数字是响应类型的分类，如 2 代表跟连接有关的响应，3 代表用户认证。
第三个数字提供了更加详细的信息。

第一个数字的含义如下：
1 表示服务器正确接收信息，还未处理。
2 表示服务器已经正确处理信息。
3 表示服务器正确接收信息，正在处理。
4 表示信息暂时错误。
5 表示信息永久错误。

第二个数字的含义如下：
0 表示语法。
1 表示系统状态和信息。
2 表示连接状态。
3 表示与用户认证有关的信息。
4 表示未定义。
5 表示与文件系统有关的信息。
*/
int MyFtpClient::login()
{
	if ( 0 >= m_iInitialized )
	{
		LOG_ERROR("ftp has not been initialized");
		return 0;
	}

	if ( 0 > m_login )
	{
		LOG_ERROR("ftp has been login");
		return 0;
	}

	recvFromServer();	//mabye some other data influense the communication

	sendToServer("USER", m_strUser);
	recvFromServer();

	sendToServer("PASS", m_strPwd);
	int ret = recvFromServer();

	if ( ret >= 0 && m_buffer[0] == '2')
	{
		m_login = 1;
		m_strCurrentPath = pwd();
		LOG_INFO("login to %s sucess", m_strServerIp.c_str() );
		return 0;
	}
	else
	{
		LOG_INFO("login to %s failed", m_strServerIp.c_str() );
		return -1;
	}
}

void MyFtpClient::logout()
{
	sendToServer("QUIT", "");
	recvFromServer();
	close(m_iDataSock);
	m_iDataSock = -1;
	m_login = -1;
	LOG_INFO("ftp client logout");
}

void split(const string& strSrc, const string& strSplit, vector<string>& vecResults)
{
	size_t found = strSrc.find_first_of(strSplit);
	if ( string::npos == found )
	{
		vecResults.push_back(strSrc);
		return ;
	}
	//size_t pos = 0;
	size_t pos_not = 0;
	while ( string::npos != found )
	{
		string tmp = strSrc.substr(pos_not, found-pos_not);
		vecResults.push_back(tmp);
		pos_not = strSrc.find_first_not_of(strSplit, found+1);
		if ( string::npos == pos_not )
		{
			return ;
		}
		found = strSrc.find_first_of(strSplit, pos_not+1);
	}
	vecResults.push_back(strSrc.substr(pos_not));
}

int MyFtpClient::createDataSocket()
{
	if ( 0 >= m_login )
	{
		// logout();
		login();
		if ( 0 >= m_login )
		{
			LOG_INFO("ftp client login failed");
			return -1;
		}
	}

	sendToServer("PASV", "");
	int ret = recvFromServer();
	if ( ret < 0 || m_buffer[1] != '2' )
	{
		LOG_ERROR("ftp create socket error");
		return -1;
	}

	m_iDataSock = socket(AF_INET, SOCK_STREAM, 0);
	if ( m_iDataSock < 0)
	{
		LOG_ERROR("ftp create socket error");
		return 1;
	}

	// (127,0,0,1,4,1)
	int i = 0;
	while ( m_buffer[i] != '(')	i++;	//i points to '('
	int j = i;
	while ( m_buffer[j] != ')')	j++;	//j points to ')'

	char ch[ j - i - 1 ];
	strncpy(ch, m_buffer+i+1, j-i-1);
	string strSrc(ch);
	vector<string> vec;
	split(strSrc, ",", vec);
	if ( vec.size() < 6 )
	{
		LOG_ERROR("the data is wrong");
		return -1;
	}
	string ip = vec[0] + "." + vec[01] + "." + vec[2] + "." + vec[3];
	int port = atoi(vec[4].c_str()) * 256 + atoi(vec[5].c_str());

	// struct hostent* dataHost = gethostbyname(ip.c_str());
	struct sockaddr_in dataServer;
	dataServer.sin_family = AF_INET;
	inet_aton(ip.c_str(), &dataServer.sin_addr);
	dataServer.sin_port = htons(port);

	ret = connect(m_iDataSock, (sockaddr*)&dataServer, sizeof dataServer);
	if ( ret < 0 )
	{
		LOG_ERROR("data socket connect to ftp server error when send PASV");
		m_iDataSock = -1;
		return -1;
	}

	LOG_INFO("data socket connect to ftp server success when send PASV");
	return 0;
}

void MyFtpClient::get(const string& serverFile, const string& localFile)
{
	int ret = createDataSocket();
	if ( ret != 0 )
	{
		closeDataSocket();
		return;
	}

	sendToServer("TYPE", "I");
	recvFromServer();

	sendToServer("RETR", "serverFile");
	recvFromServer();

	if ( m_buffer[0] == '1' )
	{
		FILE* file = fopen(localFile.c_str(), "w");
		if ( file == NULL )
		{
			LOG_ERROR("open local file error");
			return;
		}

		int pfile = fileno(file);
		while ( ( ret=read(m_iDataSock, m_dataBuffer, DATABUFSIZE)) > 0 )
		{
			write(pfile, m_dataBuffer, ret);
		}

		// closeDataSocket();
		fclose(file);

		// recvFromServer();
	}
	else
	{
		LOG_INFO("ftp client get error");
		closeDataSocket();
	}

	return;
}

int MyFtpClient::put(const string& localFile)
{
	string serverFile = localFile;
	size_t pos = localFile.find_last_of('/');
	if ( pos != string::npos )
	{
		serverFile = m_strCurrentPath + localFile.substr(pos+1);
	}

	return put(localFile, serverFile);
}

//上传文件
int MyFtpClient::put(const string& localFile, const string& serverFile)
{
	LOG_INFO("ftpclient read to put %s", localFile.c_str(), serverFile.c_str() );
	int ret = createDataSocket();
	if ( ret != 0 )
	{
		closeDataSocket();
		return -1;
	}

	if ( m_login <= 0 )
	{
		ret = login();
		if ( 0 >= m_login )
		{
			return -1;
		}
	}

	sendToServer("TYPE", "I");
	recvFromServer();

	sendToServer("STOR", serverFile);
	recvFromServer();

	if ( m_buffer[0] == '1' )
	{
		FILE* pFile = fopen(localFile.c_str(), "rb");
		if ( pFile == NULL )
		{
			LOG_ERROR("open local file error");
			return -1;
		}

		fseek(pFile, -1L, SEEK_SET);
		while ( !feof(pFile) )
		{
			bzero(m_dataBuffer, DATABUFSIZE);
			ret = fread(m_dataBuffer, 1, DATABUFSIZE, pFile);
			if ( ret <= 0 )
			{
				break;
			}
	 
	 		ret = send(m_iDataSock, m_dataBuffer, ret, 0);
			if ( ret < 0 )
			{
				// closeDataSocket();
				return ret ;
			}
		}

		fclose(pFile);

		// recvFromServer();
		// closeDataSocket();
		LOG_INFO("ftpclient put %s to %s success", localFile.c_str(), serverFile.c_str() );
		return 0;
	}
	else
	{
		LOG_ERROR("ftp client put %s to %s failed", localFile.c_str(), serverFile.c_str() );
		return -1;
	}
}

void MyFtpClient::closeDataSocket()
{
	close(m_iDataSock);
	m_iDataSock = -1;
	return;
}

string MyFtpClient::pwd()
{
	sendToServer("PWD", "");
	int ret = recvFromServer();
	if ( ret < 0 || m_buffer[0] != '2' )
	{
		LOG_INFO("PWD error");
		return "";
	}

	int i = 0;
	while (m_buffer[i++] == '"')
	{
		;
	}
	
	int j = i;
	while (m_buffer[j++] == '"')
	{
		;
	}

	char ch[j-i-1];
	strncpy(ch, m_buffer+i, j-i-1);
	string str(ch);
	return str;
}

bool MyFtpClient::cd(const string& dstPath)
{
	if ( dstPath.empty() )
	{
		return false;
	}

	string strDst = dstPath;
	if ( dstPath.at(0) != '/' )
	{
		strDst += m_strCurrentPath + dstPath;
	}
	
	sendToServer("CWD", strDst);
	int ret = recvFromServer();
	if ( ret > 0 && m_buffer[0] == '2' )
	{
		LOG_INFO("change directory: %s", strDst.c_str());
		m_strCurrentPath = strDst;
		return true;
	}

	LOG_ERROR("change directory error: %s", strDst.c_str());
	return false;
}

/*
void MyFtpClient::dir(string destdir)
{
	int res = createDataSocket();
	if (res != 0)
	{
		closeDataSocket();
		return;
	}

	int rval;
	int length = 0;
	if (destdir.length() > 1 && destdir[destdir.length() - 1] == '/')
	{
		length = destdir.length() - 1;
	}
	else
		length = destdir.length();
	memcpy(buf, "LIST ", 5);
	memcpy((char*)&buf[5], (char*)&destdir[0], length);
	buf[length + 5] = '\n';
	printf("%s\n", buf);

	rval = send(m_iCmdSock, buf, length + 6, 0);
	if (rval < 0)
	{
		//cout << "error occurred when sending LIST message to server" << endl;
		closeDataSocket();
		return;
	}

	rval = recv(m_iCmdSock, buf, BUFSIZE, 0);
	buf[rval] = '\0';
	printf("%s", buf);


	if (buf[0] == '1')
	{
		memset(dataBuf, 0, DATABUFSIZE);
		int  pos = 0;
		while ((rval = recv(m_iDataSock, &dataBuf[pos], DATABUFSIZE, 0)) > 0)
		{
			sleep(1);
			//dataBuf[rval] = '\0';
			pos += rval;
		}
		dataBuf[pos] = '\0';
		analysisFileName(dataBuf);
		closeDataSocket();

		rval = recv(m_iCmdSock, buf, BUFSIZE, 0);
		buf[rval] = '\0';

		printf("%s", buf);
		return;
	}
	else
	{
		//cout << "the server can not get prepared for data transformation" << endl;
		closeDataSocket();
		return;
	}

	return;
}

void MyFtpClient::analysisFileName(char *data)
{
	char fileInfo[2048];
	memset(fileInfo, 0, 2048);
	memcpy(fileInfo, data, strlen(data) + 1);
	int i = 0;
	int j = 0;
	printf("analysisFileName \n%s", fileInfo);
	while (fileInfo[i])
	{
		if (fileInfo[i] == '\n' && fileInfo[i - 5] == '.')
		{
			memcpy(&comtradeFileName[fileNo][0], &fileInfo[i - FILENAMESIZE - 1], FILENAMESIZE);
			comtradeFileName[fileNo][12] = '\0';
			printf("%s %d\n", comtradeFileName[fileNo], fileNo);
			fileNo++;
		}

		i++;
	}
}
void MyFtpClient::ascii()
{
	int rval;

	memcpy(buf, "TYPE A\r\n", 8);
	rval = send(m_iCmdSock, buf, 8, 0);
	if (rval < 0)
	{
		//cout << "error occurred when sending TYPE message to server" << endl;
		closeDataSocket();
		return;
	}

	rval = recv(m_iCmdSock, buf, BUFSIZE, 0);
	buf[rval] = '\0';
	printf("%s", buf);
}

string MyFtpClient::getDestdir(char* word)
{
	string destdir;
	string temp = string(word);
	if (word[0] == '/') {
		destdir = temp;
	}
	else if (temp.length() >= 2 && word[0] == '.' && word[1] == '/') {//case ./
		int length = 0, half = 0;
		if (curdir[curdir.length() - 1] != '/') {
			half = curdir.length();//half is the position to write the last half
			length = curdir.length() + temp.length() - 1;
		}
		else {
			half = curdir.length() - 1;
			length = curdir.length() + temp.length() - 2;
		}
		char* cdestdir = new char[length + 1];
		memcpy(cdestdir, (char*)&curdir[0], curdir.length());
		memcpy((char*)&cdestdir[half], (char*)&temp[1], temp.length() - 1);
		cdestdir[length] = '\0';
		destdir = string(cdestdir);
	}
	else if (temp.length() >= 3 && word[0] == '.' && //case ../
		word[1] == '.' && word[2] == '/') {
		if (curdir == "/") {
			//cout << "you are at the root directory now." << endl;
		}
		else {
			int length = 0, half = 0, i;
			if (curdir[curdir.length() - 1] != '/') {
				i = curdir.length() - 1;
			}
			else {
				i = curdir.length() - 2;
			}
			while (curdir[i] != '/') {
				i--;
			}
			half = i + 1;
			length = i + temp.length() - 2;
			char* cdestdir = new char[length + 1];
			memcpy(cdestdir, (char*)&curdir[0], curdir.length());
			memcpy((char*)&cdestdir[half], (char*)&temp[3], temp.length() - 2);
			cdestdir[length] = '\0';
			destdir = string(cdestdir);
		}
	}
	else {
		//destdir = string("**illegal");
		int length = 0, half = 0;
		if (curdir[curdir.length() - 1] != '/') {
			half = curdir.length();//half is the position to write the last half
			length = curdir.length() + temp.length() + 1;
		}
		else {
			half = curdir.length() - 1;
			length = curdir.length() + temp.length();
		}
		char* cdestdir = new char[length + 1];
		memcpy(cdestdir, (char*)&curdir[0], curdir.length());
		cdestdir[half] = '/';
		memcpy((char*)&cdestdir[half + 1], (char*)&temp[0], temp.length());
		cdestdir[length] = '\0';
		destdir = string(cdestdir);

	}

	return destdir;
}


bool MyFtpClient::main_open(char * address, char* user, char* pass, const char * port)
{

	if (connectToServer() == -1)
		return false;

	int len = recv(m_iCmdSock, buf, BUFSIZE, 0);
	if (len > 0 && buf[0] == '2')
	{
		return login();
	}
	return true;
}


void MyFtpClient::main_get(char * serverfile, char * localfile)
{
	string serverFile = getDestdir(serverfile);

	string localFile;

	int i = serverFile.length() - 1;
	while (serverFile[i] != '/')
		i--;

	char tempdir[ROOTDIRLENGTH];
	char* rootdir = getcwd(tempdir, 100);
	string srootdir = (string)rootdir;
	int length = serverFile.length() - i + srootdir.length();
	localFile = (string)localfile;
	//printf("%d\n",serverFile.length());
	get(serverFile, localFile);
}
*/
