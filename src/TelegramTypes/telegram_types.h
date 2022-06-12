#pragma once

#include <QString>

struct Document {
    int file_size;
    QString file_name, mime_type, file_id;

    bool isEmpty() const { return file_id.isEmpty(); }
};

struct File {
    QString file_id, file_path;
    int file_size;
};

struct User {
    int id{-1};
    QString username;
};

struct Message {
    int message_id{-1};
    bool is_bot_command;
    User user;
    Document document;
    QString photo_id, text, caption, filename;

    bool isEmpty() const { return text.isEmpty() && document.isEmpty() && photo_id.isEmpty(); }
};

struct Update {
    int update_id{-1};
    Message message;
};
