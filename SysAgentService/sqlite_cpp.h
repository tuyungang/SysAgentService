#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sqlite3.h>
#include <exception>
#include <map>
#include "threadlock.h"

using namespace std;

#define DBNAME "test.db"

class SqlitePlusPlus
{
    public:
        SqlitePlusPlus(const char *path);
        ~SqlitePlusPlus();

    public:
        void open(const char *path);
        void prepare(const char *sql);
        void prepare_v2();
        int  step();
        void reset();
        void exec(const char *sql);
        void finalize();
        void close();
        void s_bind_text(const char *str, int num);
        void s_bind_int(int n, int num);
        void s_column_int(int &id, int column);
        char* s_column_text(int column);

        int callback_selectAll(void *params, int n_clumn, char **column_value, char **column_name);

    private:
        sqlite3 *db;
        sqlite3_stmt *stmt;
        
};

class DatabaseManage
{
    public:
        static DatabaseManage *Instance(const char *path)
        {
            CThreadMutex mutex;
            if(pInstance == NULL){
                mutex.lock();
                if(pInstance == NULL){
                    pInstance = new DatabaseManage(path);
                }
                mutex.unlock();
            }
            return pInstance;
        }

    private:
        DatabaseManage(const char *path);
        ~DatabaseManage();
        //DatabaseManage(const DatabaseManage&);
        //DatabaseManage& operator =(const DatabaseManage&);

    public:
        class FreeInstance
        {
            public:
                ~FreeInstance()
                {
                    if (DatabaseManage::pInstance) {
                        delete DatabaseManage::pInstance;
                    }
                }

        };
        static FreeInstance _free;

    private:
        static DatabaseManage *pInstance;
        SqlitePlusPlus *_sqlite;

    public:
        void OpenDatabase(const char *path);
        void SqlPrepare(const char *sql, map<string,string> &out);
        int SqlPrepareParam(const char *sql,const char *in_param1, const char *in_param2, map<string,string> &out, const char *condition);
        void SqlExecute(const char *sql);
        void SQLFinalize();
        void CloseDatabase();

};
