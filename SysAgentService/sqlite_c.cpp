#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "sqlite_cpp.h"

extern "C"
{
    static DatabaseManage *pSqlite = NULL;  

    int opendatabase(const char *path);
    char** getinfofromblacklist(const char *sql, int &num);
    void getnumbersofrowsfromblacklist(int &sum);
    void addnewintoblacklist(const char *ip, const char *reason);
    void deleteenpiredinblacklist(const char *ip);
    int checkenpiredinblacklist();
    void closedatabase();

    int opendatabase(const char *path)
    {
        pSqlite = DatabaseManage::Instance(path);
        return 0;
    }

    char** getinfofromblacklist(const char *sql, int &num)
    {
        map<string,string> buffer;
        map<string,string>::iterator It;
        char **temp = (char**)malloc(sizeof(char*)*100);
        int i = 0;
        pSqlite->SqlPrepare(sql, buffer);
        for (It = buffer.begin(); It != buffer.end(); It++) {
            temp[i] = const_cast<char*>((*It).first.c_str());
            num = i;
            i++;
        }
        return temp;

    }

    void getnumbersofrowsfromblacklist(int &sum)
    {
        map<string,string> buffer;
        sum = pSqlite->SqlPrepareParam("select count() from mytable;", NULL, NULL, buffer, "count()");

    }

    void addnewintoblacklist(const char *ip, const char *reason)
    {
        map<string,string> buffer;
        pSqlite->SqlPrepareParam("insert into mytable(ip_address,forbidden_reason) values(?,?);", ip, reason, buffer, NULL);

    }

    void deleteenpiredinblacklist(const char *ip)
    {
        map<string,string> buffer;
        pSqlite->SqlPrepareParam("delete from mytable where ip_address=?;", ip, NULL, buffer, NULL);

    }

    int checkenpiredinblacklist()
    {

    }

    void closedatabase()
    {
        pSqlite->CloseDatabase();
    }

}
