#include "myDbSqlite.h"
#include "myLog.h"

MyDbSqlite::MyDbSqlite(string fileNameWithPath) : m_fileNameWithPath(fileNameWithPath)
{
    m_initialized = -1;
    m_db_sqlite = NULL;
    init();
}

MyDbSqlite::~MyDbSqlite()
{
    sqlite3_close(m_db_sqlite);
}

int MyDbSqlite::init()
{
    if ( 0 < m_initialized )
    {
        return 0;
    }

    int ret = sqlite3_open(m_fileNameWithPath.c_str(), &m_db_sqlite);
    if ( SQLITE_OK != ret )
    {
        LOG_ERROR( "sqlite3_open error: %d", ret );
        sqlite3_close(m_db_sqlite);
        return -1;
    }

    LOG_INFO("init my db of sqlite success, %s", m_fileNameWithPath.c_str());
    m_initialized = 1;
    return 0;
}

vector< vector<std::string> > MyDbSqlite::getResults()
{
    return m_vec_rsts;
}

int callback(void* arg, int argc, char* argv[], char* fieldName[])
{
    if ( !arg )
    {
        return 0;
    }
    vector< vector<string> >& vec_vec = ( (MyDbSqlite*)arg )->m_vec_rsts;
    vector<string> vec;
    string tmp;
    for ( int i=0; i<argc; i++ )
    {
        tmp = argv[i];
        vec.push_back(tmp);
        // printf("%s = %s\n", fieldName[i], argv[i] ? argv[i] : "NULL");
    }
    vec_vec.push_back(vec);
    return 0;
}

int MyDbSqlite::execute(string& strSql)
{
    if ( strSql.empty() )
    {
        return -1;
    }
LOG_INFO("sql: %s", strSql.c_str());
    int ret = -1;
    int row = 0;
    int col = 0;
    char* errMsg = NULL;
    char** ppResults = NULL;
    string strOut;
    if ( strSql[0] == 's' || strSql[0] == 'S' )
    {
        m_vec_rsts.clear();
        ret = sqlite3_exec(m_db_sqlite, strSql.c_str(), callback, this, &errMsg);
        // ret = sqlite3_get_table(m_db_sqlite, strSql.c_str(), &ppResults, &row, &col, &errMsg);
        if ( ret != SQLITE_OK )
        {
            LOG_ERROR("sqlite3_get_table error: %s", errMsg);
        }
        else if ( 0 )
        {
            string strOut;
            //int nIndex = col;
            for ( int i=0; i<row; ++i )
            {
                for ( int j=0; j<col; ++j )
                {
                    // 
                }
            }
            cout << strOut << endl;
        }
    }
    else
    {
        ret = sqlite3_exec(m_db_sqlite, strSql.c_str(), NULL, NULL, &errMsg);
        if ( ret != SQLITE_OK )
        {
            LOG_ERROR("sqlite3_exec error: %s", errMsg);
            // sqlite3_free(errMsg);
        }
    }

    sqlite3_free_table(ppResults);
    sqlite3_free(errMsg);
    return ret;
}

void MyDbSqlite::display()
{
    for ( auto val : m_vec_rsts )
    {
        for ( auto var : val )
        {
            cout << var << '\t';
        }
        cout << endl;
    }
}
