#include "sqlitedb.h"

SqliteDB::SqliteDB()
{

}

void SqliteDB::createDB(const QString &dbName, const QString &userName, const QString password)
{
    if (QSqlDatabase::contains("qt_default_connection")) {
        db = QSqlDatabase::database("qt_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbName);
        db.setUserName(userName);
        db.setPassword(password);
    }
    return;
}

void SqliteDB::createTable(const QString &sql)
{
    QSqlQuery query;
    query.prepare(sql);
    if (query.exec()) {

    } else {

    }
    return;
}

bool SqliteDB::open()
{
    return db.open();
}

void SqliteDB::close()
{
    return db.close();
}
