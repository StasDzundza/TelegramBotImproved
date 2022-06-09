#include "telegram_types_factory.h"

#include <QJsonArray>
#include <QJsonDocument>

QVector<Update> TelegramTypesFactory::parseUpdates(const QJsonObject& updates) {
    QVector<Update> updates_vector;
    const QJsonArray& result = updates["result"].toArray();
    foreach (auto&& json_value, result) {
        const QJsonObject update_json_object = json_value.toObject();
        Update update = createUpdate(update_json_object);
        updates_vector.push_back(std::move(update));
    }
    return updates_vector;
}

Update TelegramTypesFactory::createUpdate(const QJsonObject& update_json_object) {
    Update update;
    const int update_id = update_json_object["update_id"].toInt();
    const Message& message = createMessage(update_json_object["message"].toObject());
    return { update_id, message };
}

Message TelegramTypesFactory::createMessage(const QJsonObject& message_json_object) {
    const int message_id = message_json_object["message_id"].toInt();
    const QString& text = message_json_object["text"].toString();
    const QString& caption = message_json_object["caption"].toString();
    const User& user = createUser(message_json_object["from"].toObject());
    const Document& doc = createDocument(message_json_object["document"].toObject());
    // parse photo
    const QJsonArray& photos = message_json_object["photo"].toArray();
    const int best_quality_photo_index = photos.size() - 1;
    const QJsonObject& photo_json_object = photos.at(best_quality_photo_index - 1).toObject();

    Message message;
    message.message_id = message_id;
    message.user = user;
    message.document = doc;
    message.text = text;
    message.caption = caption;
    message.photo_id = photo_json_object["file_id"].toString();
    return message;
}

User TelegramTypesFactory::createUser(const QJsonObject& user_json_object) {
    const int id = user_json_object["id"].toInt();
    const QString& username = user_json_object["username"].toString();
    return { id, username };
}

File TelegramTypesFactory::createFile(const QJsonObject& file_json_object) {
    const QJsonObject& file_object = file_json_object["result"].toObject();
    const QString& file_id = file_object["file_id"].toString();
    const QString& file_path = file_object["file_path"].toString();
    const int file_size = file_object["file_size"].toInt();
    return { file_id, file_path, file_size };
}

Document TelegramTypesFactory::createDocument(const QJsonObject& document_json_object) {
    const QString& file_name = document_json_object["file_name"].toString();
    const QString& mime_type = document_json_object["mime_type"].toString();
    const QString& file_id = document_json_object["file_id"].toString();
    const int file_size = document_json_object["file_size"].toInt();
    return { file_size, file_name, mime_type, file_id };
}

QString TelegramTypesFactory::buildJsonCommandKeyboardObject(const QVector<QString>& buttons) {
    QJsonObject keyboard_json;
    QJsonArray keyboard_buttons;
    for (const auto& command : buttons) {
        QJsonObject button;
        button.insert("text", command);
        QJsonArray button_array;
        button_array.push_back(button);
        keyboard_buttons.push_back(button_array);
    }
    keyboard_json.insert("keyboard", keyboard_buttons);
    QJsonDocument doc(keyboard_json);
    return doc.toJson();
}
