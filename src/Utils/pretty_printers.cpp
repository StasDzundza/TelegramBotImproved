#include "pretty_printers.h"

#include <sstream>

QString toString(const Message& message) {
    std::stringstream info;
    info << "Message id = " << message.message_id << '\n'
         << "Message text : " << message.text.toStdString() << '\n'
         << "User info : " << toString(message.user).toStdString() << '\n';
    return QString::fromStdString(info.str());
}

QString toString(const Update& update) {
    std::stringstream info;
    info << "Update id = " << update.update_id << '\n'
         << "Message info : " << toString(update.message).toStdString() << '\n';
    return QString::fromStdString(info.str());
}

QString toString(const User& user) {
    return "User id = " + QString::number(user.id) + " and username is : " + user.username;
}
