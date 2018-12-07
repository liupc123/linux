#include "myDbMysql.h"
#include "myLog.h"

MyDbMysql::MyDbMysql(string db_host, string db_user, string db_pwd, string db_name) :
    m_db_host(db_host),
    m_db_user(db_user),
    m_db_pwd(db_pwd),
    m_db_name(db_name)
{
    m_initialized = 0;
    m_vec_rsts.clear();
    m_db_rsts = NULL;
    init();
}

MyDbMysql::~MyDbMysql()
{
    if ( m_db_rsts )
    {
        mysql_free_result(m_db_rsts);
        m_db_rsts = NULL;
    }
    mysql_close(&m_db_mysql);
    mysql_library_end();
    mysql_server_end();
}

int MyDbMysql::init()
{
    if ( 0 != m_initialized )
    {
        return 0;
    }

    try
    {
        if ( ! mysql_init(&m_db_mysql) )
        {
            LOG_ERROR( "mysql_init error");
            return -1;
        }

        //设置编码格式
        // int ret = mysql_set_character_set(&m_db_mysql, "utf8");
        // if ( 0 != ret )
        // {
        //     LOG_ERROR( "mysql_set_character_set error: %d", ret);
        //     return -1;
        // }
        
        //函数mysql_real_connect建立一个数据库连接
        if ( ! mysql_real_connect(&m_db_mysql, m_db_host.c_str(), m_db_user.c_str(), m_db_pwd.c_str(), m_db_name.c_str(), 0, NULL, 0) )
        {
            LOG_ERROR( "mysql_real_connect error: %d, %s", mysql_errno(&m_db_mysql), mysql_error(&m_db_mysql) );
            return -1;
        }
    }
    catch(string& msg)
    {
        LOG_ERROR( "mysql error: %s", msg.c_str());
        return -1;
    }

    LOG_INFO("init my db of mysql success, %s, %s, %s, %s,", m_db_host.c_str(), m_db_user.c_str(), m_db_pwd.c_str(), m_db_name.c_str());
    m_initialized = 1;
    return 0;
}

int MyDbMysql::execute(string& srcSql)
{
    if ( srcSql.empty() || 0==m_initialized )
    {
        LOG_INFO("initialized: %d, sql: %s", m_initialized, srcSql.c_str() );
        // return -1;
    }
    int ret = 0;
    LOG_INFO("execute sql: %s", srcSql.c_str());
    try
    {
        // mysql_options(&m_db_mysql, MYSQL_SET_CHARSET_NAME, "gb2312");
        // mysql_query(&m_db_mysql, "SET NAMES GBK");  //设置编码格式

        // string strSqlDst;
        // ret = mysql_real_escape_string(&m_db_mysql, const_cast<char*>(srcSql.c_str()), strSqlDst.c_str(), srcSql.length()*2+1 );
        // if( 0 != ret )
        // {
        //     LOG_ERROR("mysql_real_escape_string error: %d", ret);
        //     return -1;
        // }

        //mysql_query()执行成功返回0,执行失败返回非0值
        ret = mysql_query(&m_db_mysql, srcSql.c_str());
        if( 0 != ret )
        {
            LOG_ERROR("mysql_query error: %d", ret);
            return -1;
        }
        else    //查询成功
        {
            m_db_rsts = mysql_store_result(&m_db_mysql); //获取结果集
            if( m_db_rsts ) //返回结果集
            {
                int num_fields = mysql_num_fields(m_db_rsts);      //获取结果集中总共的字段树，即列树
                int num_rows = mysql_num_rows(m_db_rsts);          //获取结果集中总共的行数
                m_vec_rsts.clear();
                for(int i=0; i<num_rows; ++i)   //输出每一行
                {
                    MYSQL_ROW m_db_row = mysql_fetch_row(m_db_rsts);  //获取下一行数据
                    if ( !m_db_row )
                    {
                        break;
                    }
                    string tmp = "";
                    for (int j=0; j<num_fields; j++)    //输出每一字段
                    {
                        string str(m_db_row[j]);
                        tmp += str + ",";
                    }
                    m_vec_rsts.push_back(tmp);
                }
                ret = num_rows;
                mysql_free_result(m_db_rsts);
                m_db_rsts = NULL;
            }
            else
            {
                ret = mysql_field_count(&m_db_mysql);
                if ( ret == 0 )  //代表执行的是update,insert,delete类的非查询语句
                {
                    // it was not a SELECT
                    ret = mysql_affected_rows(&m_db_mysql);     //返回update，insert, delete影响的行数
                }
                else
                {
                    LOG_ERROR("mysql get result error: %d", ret);
                    mysql_rollback(&m_db_mysql);
                    return ret;
                }
            }
            mysql_commit(&m_db_mysql);
            return ret;
        }
    }
    catch(string& msg)
    {
        LOG_ERROR("mysql error: %s", msg.c_str());
        return -1;
    }

    return 0;
}

void MyDbMysql::getResults(vector<std::string>& vec)
{
    vec = m_vec_rsts;
}

void MyDbMysql::display()
{
    for ( auto val : m_vec_rsts )
    {
        cout << "val: " << val << endl;
    }
}