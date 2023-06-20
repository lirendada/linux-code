#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#define HOST "127.0.0.1"  // 不允许使用公网地址，所以用本地换回
#define USER "root"
#define PASSWD ""
#define DBNAME "gobang"
#define PORT 3306

int main()
{
    // 1.初始化mysql句柄
    MYSQL* mysql = mysql_init(NULL);
    if(mysql == NULL)
    {
        printf("mysql init fail!\n");
        return -1;
    }

    // 2.连接mysql服务器
    if(mysql_real_connect(mysql, HOST, USER, PASSWD, DBNAME, PORT, NULL, 0) == NULL)
    {
        printf("mysql connect fail：%s\n", mysql_errno(mysql)); // 打印错误信息
        mysql_close(mysql); // 记得关闭句柄
        return -1;
    }

    // 3.设置客户端字符集
    if(mysql_set_character_set(mysql, "utf8") != 0)
    {
        printf("client set character fail：%s\n", mysql_errno(mysql)); // 打印错误信息
        mysql_close(mysql); // 记得关闭句柄
        return -1;
    }

    // 4.选择要操作的数据库（其实也可以不用，因为在初始化的时候已经指定了要选择的数据库）
    if(mysql_select_db(mysql, DBNAME) != 0)
    {
        printf("select database fail：%s\n", mysql_errno(mysql)); // 打印错误信息
        mysql_close(mysql); // 记得关闭句柄
        return -1;
    }

    // 5.执行sql语句
    // char* sql = "insert stu value(null, '利刃', 1314, 100, 150, 149)"; // 增
    // char* sql = "update stu set math=math-40 where stu_num=1;";        // 改
    // char* sql = "delete from stu where stu_num=1;";                    // 删
    char* sql = "select * from stu;"; // 查，需要配合下面的第6、7、8步
    int n = mysql_query(mysql, sql);
    if(n != 0)
    {
        printf("%s: sql query fail：%s\n", sql, mysql_errno(mysql)); // 打印错误信息
        mysql_close(mysql); // 记得关闭句柄
        return -1;
    }

    // 6.如果sql语句是查询语句，需要保存结果到本地
    MYSQL_RES* res = mysql_store_result(mysql);
    if(res == NULL)
    {
        mysql_close(mysql); // 记得关闭句柄
        return -1;
    }

    // 7.获取结果集中的结果条数：行数和列数
    int num_row = mysql_num_rows(res);
    int num_field = mysql_num_fields(res);

    // 8.遍历保存到本地的结果集
    for(int i = 0; i < num_row; ++i)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        for(int j = 0; j < num_field; ++j)
        {
            printf("%s\t", row[j]);
        }
        printf("\n");
    }

    // 9.释放结果集
    mysql_free_result(res);

    // 10.关闭连接，释放mysql句柄
    mysql_close(mysql);
    return 0;
}