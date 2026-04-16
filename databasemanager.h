#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>

class DatabaseManager {
public:
    static bool init() {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("passwords.db");
        if (!db.open()) return false;

        QSqlQuery query;
        return query.exec("CREATE TABLE IF NOT EXISTS accounts ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "service TEXT, "
                          "login TEXT, "
                          "password TEXT)");
    }
};

#endif
