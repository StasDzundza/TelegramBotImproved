#include "database.h"

#include <QSqlDriver>
#include <QVariant>

// clazy:skip
namespace {
const QString DB_CONNECTION_NAME = "telegram_bot_db";
}  // namespace

Database::Database() {
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    assert(db_.open());

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS UserCommands (userId INTEGER PRIMARY KEY, lastCommand TEXT)");
}

Database::~Database() { db_.close(); }

void Database::storeLastUserCommand(int user_id, const QString& last_user_command) const {
    QSqlQuery query;
    query.prepare(
        "SELECT * FROM UserCommands "
        "WHERE userId = ?");
    query.addBindValue(user_id);
    query.exec();

    if (getNumOfQueryRows(query) > 0) {  // record with user id already exists
        QSqlQuery updateQuery;
        updateQuery.prepare(
            "UPDATE UserCommands SET lastCommand = ? "
            "WHERE userId = ?");
        updateQuery.addBindValue(last_user_command);
        updateQuery.addBindValue(user_id);
        updateQuery.exec();
    } else {
        QSqlQuery insertQuery;
        insertQuery.prepare(
            "INSERT INTO UserCommands (userId, lastCommand) "
            "VALUES (?, ?)");
        insertQuery.addBindValue(user_id);
        insertQuery.addBindValue(last_user_command);
        insertQuery.exec();
    }
}

void Database::clearLastUserCommand(int user_id) const { storeLastUserCommand(user_id, "NULL"); }

QString Database::getLastUserCommand(int user_id) const {
    QSqlQuery query;
    query.prepare(
        "SELECT lastCommand FROM UserCommands "
        "WHERE userId = ?");
    query.addBindValue(user_id);
    query.exec();

    if (getNumOfQueryRows(query) > 0) {
        query.first();
        const QString& last_command = query.value(0).toString();
        return last_command == "NULL" ? "" : last_command;
    } else {
        return "";
    }
}

int Database::getNumOfQueryRows(QSqlQuery query) const {
    QSqlDatabase defaultDB = QSqlDatabase::database();
    if (db_.driver()->hasFeature(QSqlDriver::QuerySize)) {
        return query.size();
    } else {
        query.last();
        return query.at() + 1;
    }
}
