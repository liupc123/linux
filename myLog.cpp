#include "myLog.h"


MyLog* MyLog::m_instance = NULL;

const string strFileName_ = "./my.log";
MyLog::MyLog()
{
    m_bInitialised = false;
    m_strFileName = strFileName_;
    m_eMyLogType = LOG_TYPE_DEBUG;
    m_eMyLogOutput = LOG_OUTPUT_FILE;
}

int MyLog::init( const std::string& strFileName, eMyLogOutput eOutput )
{
    m_strFileName = strFileName;
    m_eMyLogOutput = eOutput;
    return init();
}

int MyLog::init( const std::string& strFileName, string strOutput )
{
    m_strFileName = strFileName;
    if ( "1" == strOutput )
    {
        m_eMyLogOutput = LOG_OUTPUT_COUT;
    }
    else if ( "2" == strOutput )
    {
        m_eMyLogOutput = LOG_OUTPUT_FILE;
    }
    else
    {
        return -1;
    }
    return init();
}

int MyLog::init()
{
    if( !m_bInitialised )
    {
        if ( LOG_OUTPUT_FILE == m_eMyLogOutput )
        {
            m_ofStream.open( m_strFileName.c_str(), std::ios_base::app );
            if ( m_ofStream.is_open() )
            {
                m_bInitialised = true;
                return 0;
            }
        }
    }
    return -1;
}

MyLog::~MyLog()
{
    // if ( m_bInitialised )
    {
        m_ofStream.flush();
        m_ofStream.close();
        m_bInitialised = false;
    }
}

string getStrTime()
{
    time_t t = time(0); 
    char tmp[32]; 
    strftime( tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t) );
    return tmp;
}

MyLog* MyLog::getIns()
{
    if ( NULL == m_instance )
    {
        m_instance = new MyLog();
    }
    return m_instance;
}

int MyLog::write(const char* format, ... )
{
    char buffer[iMyLogDataSize];
    va_list varArgs;
    va_start( varArgs, format );
    vsnprintf( buffer, sizeof(buffer), format, varArgs);
    va_end( varArgs );

    if ( LOG_OUTPUT_COUT == m_eMyLogOutput )
    {
        cout << buffer;
        cout.flush();
    }
    else if ( LOG_OUTPUT_FILE == m_eMyLogOutput )
    {
        if ( m_ofStream.is_open() )
        {
            m_ofStream << buffer;
            m_ofStream.flush();
        }
    }
    return 0;
}
