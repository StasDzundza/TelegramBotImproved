#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>

#include "singleton.h"

class Database final : public Singleton<Database> {
   public:
    Database();
    ~Database();

    void storeLastUserCommand(int user_id, const QString& last_user_command) const;
    void clearLastUserCommand(int user_id) const;
    QString getLastUserCommand(int user_id) const;

   private:
    QSqlDatabase db_;

    int getNumOfQueryRows(QSqlQuery query) const;
};
