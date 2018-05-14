#include "sqlite_cpp.h"

DatabaseManage* DatabaseManage::pInstance = NULL;

DatabaseManage::DatabaseManage(const char *path) : _sqlite(NULL)
{
    _sqlite = new SqlitePlusPlus(path);

}

DatabaseManage::~DatabaseManage()
{
    delete _sqlite;
}

void DatabaseManage::OpenDatabase(const char *path)
{
    _sqlite->open(path);

}

void DatabaseManage::SqlPrepare(const char *sql, map<string,string> &out)
{
    _sqlite->prepare(sql);
    int res = _sqlite->step();
    if (res == SQLITE_DONE) {
        return;
    }
    while (res == SQLITE_ROW) {
        out.insert(map<string,string>::value_type(_sqlite->s_column_text(1),_sqlite->s_column_text(2)));
        res = _sqlite->step();
    }
    _sqlite->finalize();
}

int DatabaseManage::SqlPrepareParam(const char *sql,const char *in_param1, const char *in_param2, map<string,string> &out, const char *condition)
{
    int sum = 0;
    _sqlite->prepare(sql);
    if (in_param1 != NULL && in_param2 != NULL) {
        _sqlite->s_bind_text(in_param1, 1);
        _sqlite->s_bind_text(in_param2, 2);
    }
    else if (in_param1 != NULL && in_param2 == NULL){
        _sqlite->s_bind_text(in_param1, 1);
    }
    else if (in_param2 != NULL && in_param1 == NULL) {
        _sqlite->s_bind_text(in_param2, 1);
    }
    else if (condition != NULL) {
        //_sqlite->s_bind_int(3, 1);
    }

    int res = _sqlite->step();
    if (res == SQLITE_DONE) {
        return sum;
    }
    while (res == SQLITE_ROW) {
        if (strcmp(condition, "count()") == 0) {
             _sqlite->s_column_int(sum, 0);
        }
        else {
            out.insert(map<string,string>::value_type(_sqlite->s_column_text(1),_sqlite->s_column_text(2)));

        }
        res = _sqlite->step();

    }
    _sqlite->reset();
    _sqlite->finalize();

    return sum;
}

void DatabaseManage::SqlExecute(const char *sql)
{
    _sqlite->exec(sql);
}

void DatabaseManage::SQLFinalize()
{
    _sqlite->finalize();
}

void DatabaseManage::CloseDatabase()
{
    _sqlite->close();

}

SqlitePlusPlus::SqlitePlusPlus(const char *path) : db(NULL), stmt(NULL)
{
    try{
        open(path);
    }
    catch (exception &e)
    {
        cout << e.what() << endl;
    }

}

SqlitePlusPlus::~SqlitePlusPlus()
{
    //delete db;
    //delete stmt;
}

void SqlitePlusPlus::open(const char *path)
{
    int res = sqlite3_open(path, &db);
    if (res != SQLITE_OK) {
        throw std::exception();
    }

}

void SqlitePlusPlus::prepare(const char *sql)
{
    sqlite3_prepare(db, sql, -1, &stmt, NULL);
}

void SqlitePlusPlus::prepare_v2()
{

}

int SqlitePlusPlus::step()
{
    return sqlite3_step(stmt);

}

void SqlitePlusPlus::reset()
{
    sqlite3_reset(stmt);

}

void SqlitePlusPlus::exec(const char *sql)
{
    sqlite3_exec(db, sql, NULL, NULL, NULL);

}

void SqlitePlusPlus::finalize()
{
    sqlite3_finalize(stmt);

}

void SqlitePlusPlus::close()
{
    sqlite3_close(db);

}

void SqlitePlusPlus::s_bind_text(const char *str, int num)
{
    sqlite3_bind_text(stmt, num, str, -1, SQLITE_STATIC);
}

void SqlitePlusPlus::s_bind_int(int n, int num)
{
    sqlite3_bind_int(stmt, num, n);

}

void SqlitePlusPlus::s_column_int(int &out_id, int column)
{
    out_id = sqlite3_column_int(stmt, column);
}

char* SqlitePlusPlus::s_column_text(int column)
{
    const unsigned char *temp = sqlite3_column_text(stmt, column);
    char *out = (char *)(const_cast<unsigned char*>(temp));
    return out;
}

int SqlitePlusPlus::callback_selectAll(void *params, int n_clumn, char **column_value, char **column_name)
{

}

