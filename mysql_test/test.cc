#include <iostream>
#include <mysql/mysql.h>
using namespace std;

int main() 
{
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned long *lengths;

    conn = mysql_init(NULL);
    if (conn == NULL) 
    {
        fprintf(stderr, "mysql_init failed\n");
        return 1;
    }

    if (mysql_real_connect(conn, "localhost", "liren", "TThh1314520@51gsd4fs", "scott", 0, NULL, 0) == NULL) 
    {
        fprintf(stderr, "mysql_real_connect failed\n");
        return 1;
    }

    if (mysql_query(conn, "SELECT ename, job FROM emp")) 
    {
        fprintf(stderr, "mysql_query failed\n");
        return 1;
    }

    result = mysql_store_result(conn);
    if (result == NULL) 
    {
        fprintf(stderr, "mysql_store_result failed\n");
        return 1;
    }

    while ((row = mysql_fetch_row(result))) 
    {
        lengths = mysql_fetch_lengths(result);

        // 处理获取到的一行数据及其长度信息
        printf("Column 1: %s (length: %lu), Column 2: %s (length: %lu)\n", row[0], lengths[0], row[1], lengths[1]);
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}
