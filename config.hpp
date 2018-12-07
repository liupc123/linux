#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <utility>

using std::ifstream;
using std::string;
using std::map;
using std::pair;
using std::cout;
using std::endl;

class Config
{
public:
	static Config* getIns();
	void init(const string strFileName);
	void init();
	void display();
	int readConfigFile();
	string getValue(const string strKey);
	~Config();

private:
	Config();
	static Config* m_pInstance;
	string m_strFileNameWithPath;
	map<string, string> m_mapValue;
};


Config* Config::m_pInstance = nullptr;
const string fileName = "./config.ini";

Config::Config() : m_strFileNameWithPath(fileName)
{
	//...
}

Config::~Config()
{
	//...
}

Config* Config::getIns()
{
	if ( NULL == m_pInstance)
	{
		m_pInstance = new Config();
	}
	return m_pInstance;
}

void Config::init(const string strFileName)
{
	m_strFileNameWithPath = strFileName;
	init();
}

void Config::init()
{
	string strPrefix = m_strFileNameWithPath.substr(m_strFileNameWithPath.length()-4,4);
	if ( strPrefix != ".ini" && strPrefix != ".INI" )
	{
		cout << m_strFileNameWithPath << " is not a \"ini\" file! " << strPrefix << endl;
		return;
	}
	
	readConfigFile();
}

void trim(string& strSrc)
{
	strSrc.erase(0, strSrc.find_first_not_of("\r\t\n "));
    strSrc.erase(strSrc.find_last_not_of("\r\t\n ")+1);
}

int Config::readConfigFile()
{
	ifstream ifs;
	ifs.open(m_strFileNameWithPath, std::ios_base::in);
	if( ! ifs.is_open())
	{
		cout<<"can not open file: "<< m_strFileNameWithPath << endl;
		return -1;
	}

	string line = "";
	string key = "";
	string value = "";
	char chTemp[100];
	while(!ifs.eof())
	{
		ifs.getline(chTemp, sizeof(chTemp));
		line = chTemp;
		size_t pos = line.find('=');
		if (pos != string::npos)
		{
			key = line.substr(0, pos);
			value = line.substr(pos+1);
			trim(key);
			trim(value);
			m_mapValue.insert( pair<string, string>(key, value) );
		}
	}
	ifs.close();
	return 0;
}

string Config::getValue(const string strKey)
{
	auto search = m_mapValue.find(strKey);
	if ( m_mapValue.end() != search )
	{
		return search->second;
	}
	else
	{
		return "";
	}
}

void Config::display()
{
	for(auto& it : m_mapValue)
    cout << "[" << it.first << " - " << it.second << "]" << endl;
}


