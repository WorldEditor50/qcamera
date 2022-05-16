#ifndef SQLITEDB_H
#define SQLITEDB_H
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

class SqliteDB
{
public:
    SqliteDB();
    void createDB(const QString &dbName, const QString &userName, const QString password);
    void createTable(const QString &sql);
    bool open();
    void close();
    void exec(const QString &sql);
private:
    QSqlDatabase db;
};

#endif // SQLITEDB_H
