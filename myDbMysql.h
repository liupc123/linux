#ifndef _MYDBMYSQL_H_
#define _MYDBMYSQL_H_

#include <iostream>
#include <string>
#include <mysql.h>
#include <mutex>
#include <vector>

using std::string;
using std::vector;

class MyDbMysql
{
public:
    MyDbMysql(string db_host, string db_user, string db_pwd, string db_name);
    ~MyDbMysql();
    int init();     //连接mysql
    int execute(string& srcSql);    //执行sql语句
    void getResults(vector<std::string>& vec);
    void display();

private:
    string m_db_host;
    string m_db_user;
    string m_db_pwd;
    string m_db_name;
    vector<std::string> m_vec_rsts;

    int m_initialized;
    MYSQL m_db_mysql;     //连接mysql句柄指针
    MYSQL_RES* m_db_rsts;   //指向查询结果的指针
    // MYSQL_ROW m_db_row;    //按行返回的查询信息
};

//CREATE TABLE `test`.`sf_msg`(    `no` BIGINT AUTO_INCREMENT COMMENT '序号',  `msg` VARCHAR(100) COMMENT '条码',  `recv_time` CHAR(14) COMMENT '收到条码时间',  `cond` INT(1) COMMENT '有图有码？0皆有，1有码，2有图，皆无',   KEY(`no`));

#endif
