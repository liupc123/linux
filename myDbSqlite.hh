#ifndef _MYDBSQLITE_H_
#define _MYDBSQLITE_H_

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>

using std::string;
using std::vector;

class MyDbSqlite
{
public:
    MyDbSqlite(string fileNameWithPath);
    ~MyDbSqlite();
    int init();     //连接mysql
    int execute(string& srcSql);    //执行sql语句
    vector< vector<std::string> > getResults();
    void display();
    vector< vector<std::string> > m_vec_rsts;

private:
    string m_fileNameWithPath;
    int m_initialized;
    sqlite3* m_db_sqlite;     //连接sqlite句柄指针
};

#endif
