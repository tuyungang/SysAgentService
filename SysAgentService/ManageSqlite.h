#pragma once
//#include <stdio.h>
//#include <stdlib.h>
//#include <iostream>
//#include <string.h>
//#include <map>
//#include "sqlite3/sqlite3.h"
//#include "Util.h"
//
//using namespace std;
//
//class SqlitePlusPlus
//{
//    public:
//        SqlitePlusPlus(const char *path);
//        ~SqlitePlusPlus();
//
//    public:
//        void open(const char *path);
//        void prepare(const char *sql);
//        void prepare_v2();
//        int  step();
//        void reset();
//        void exec(const char *sql);
//        void finalize();
//        void close();
//        void s_bind_text(const char *str, int num);
//        void s_bind_int(int n, int num);
//        void s_column_int(int &id, int column);
//        char* s_column_text(int column);
//
//        int callback_selectAll(void *params, int n_clumn, char **column_value, char **column_name);
//
//    private:
//        sqlite3 *db;
//        sqlite3_stmt *stmt;
//        
//};
//class CLock;
//class DatabaseManage
//{
//    public:
//        static DatabaseManage *Instance(const char *path)
//        {
//            if(pInstance == NULL){
//				CLock lock(cs);
//                if(pInstance == NULL){
//                    pInstance = new DatabaseManage(path);
//                }
//            }
//            return pInstance;
//        }
//
//    private:
//        DatabaseManage(const char *path);
//        ~DatabaseManage();
//        //DatabaseManage(const DatabaseManage&);
//        //DatabaseManage& operator =(const DatabaseManage&);
//
//    public:
//        class FreeInstance
//        {
//            public:
//                ~FreeInstance()
//                {
//                    if (DatabaseManage::pInstance) {
//                        delete DatabaseManage::pInstance;
//                    }
//                }
//
//        };
//        static FreeInstance _free;
//
//    private:
//        static DatabaseManage *pInstance;
//        SqlitePlusPlus *_sqlite;
//		static CRITICAL_SECTION cs;
//
//    public:
//        void OpenDatabase(const char *path);
//        void SqlPrepare(const char *sql, map<string,string> &out);
//        int SqlPrepareParam(const char *sql,const char *in_param1, const char *in_param2, map<string,string> &out, const char *condition);
//        void SqlExecute(const char *sql);
//        void SQLFinalize();
//        void CloseDatabase();
//
//};