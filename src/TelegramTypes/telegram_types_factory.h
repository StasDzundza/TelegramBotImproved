#pragma once

#include <QJsonObject>
#include <QSet>

#include "telegram_types.h"

class TelegramTypesFactory {
   public:
    TelegramTypesFactory();

    static QVector<Update> parseUpdates(const QJsonObject& updates);
    static Update createUpdate(const QJsonObject& update_json_object);
    static Message createMessage(const QJsonObject& message_json_object);
    static User createUser(const QJsonObject& user_json_object);
    static File createFile(const QJsonObject& file_json_object);
    static Document createDocument(const QJsonObject& document_json_object);
    static QString buildJsonCommandKeyboardObject(const QVector<QString>& buttons);
};
