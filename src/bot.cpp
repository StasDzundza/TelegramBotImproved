#include "bot.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QPointer>
#include <iostream>

#include "file_downloader.h"
#include "pretty_printers.h"
#include "telegram_types_factory.h"
#include "tesseract_ocr.h"
#include "text_utils.h"
#include "translater.h"

// clazy:skip
namespace {
const QString TELEGRAM_API_URL = "https://api.telegram.org/";

const QString INVALID_COMMAND =
    "INVALID COMMAND. Write /commands and check valid and existing "
    "commands.";
const QString INVALID_TYPE_OF_TEXT_FILE = "Invalide type of file. It should be text file, not image.";
const QString INVALID_TYPE_OF_PHOTO_FILE = "Invalide type of file. It should be image.";
const QString DESCRIPTION =
    "This is awesome bot developed by @dzundza_stas, which can recognize "
    "printed and handwrited text,"
    "translate it to different languages and send result in text and file "
    "formats. Write /start command for "
    "start using it.";
const QString ABOUT =
    "Developed by Stanislav Dzundza(@dzundza_stas) during studying in "
    "Taras "
    "Shevchenko National University of Kyiv.";

const QSet<QString> VALID_COMMANDS{
    "/translate_text",      "/recognize_photo", "/translate_photo", "/translate_file", "/write_text_to_file",
    "/supported_languages", "/commands",        "/description",     "/about",          "/start"};
const QVector<QString> KEYBOARD_BUTTONS{"/translate_text", "/recognize_photo",    "/translate_photo",
                                        "/translate_file", "/write_text_to_file", "/supported_languages",
                                        "/commands",       "/description",        "/about"};
const QVector<QString> SUPPORTED_LANGUAGES{"English", "Ukrainian", "Russian"};
}  // namespace

Bot::Bot() : BOT_TOKEN(qgetenv("BOT_TOKEN")) {
    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    update_request_.setSslConfiguration(sslConfiguration);
    send_request_.setSslConfiguration(sslConfiguration);

    connect(&update_access_manager_, &QNetworkAccessManager::finished, this, &Bot::receiveUpdates);
    connect(&send_access_manager_, &QNetworkAccessManager::finished, this, &Bot::receiveSendingResult);
}

void Bot::start() {
    update_request_.setUrl(QUrl(TELEGRAM_API_URL + "bot" + BOT_TOKEN + "/getUpdates"));
    update_access_manager_.get(update_request_);
}

QString Bot::getToken() const { return BOT_TOKEN; }

void Bot::receiveUpdates(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray& telegram_answer = reply->readAll();
        std::cout << telegram_answer.toStdString() << std::endl;
        const QJsonDocument& json_document = QJsonDocument::fromJson(telegram_answer);
        const QJsonObject& rootObject = json_document.object();
        const QVector<Update>& updates = TelegramTypesFactory::parseUpdates(rootObject);
        int newest_update_id = -1;
        foreach (const Update& update, updates) {
            if (update.update_id > newest_update_id) {
                newest_update_id = update.update_id;
            }
            processUpdate(update);
        }
        update_request_.setUrl(
            QUrl(TELEGRAM_API_URL + "bot" + BOT_TOKEN + "/getUpdates?offset=" + QString::number(newest_update_id + 1)));
        update_access_manager_.get(update_request_);
    }
}

void Bot::processUpdate(const Update& update) {
    const Message& updateMessage = update.message;
    if (!updateMessage.isEmpty()) {
        const int user_id = updateMessage.user.id;
        if (VALID_COMMANDS.contains(updateMessage.text) ||  // check if commannd is valid
            (last_user_commands_.contains(user_id) && !last_user_commands_[user_id].isEmpty())) {
            if (last_user_commands_.contains(user_id) && !last_user_commands_[user_id].isEmpty() &&
                !VALID_COMMANDS.contains(updateMessage.text)) {
                executeUserCommand(update);
            } else {
                last_user_commands_[user_id] = updateMessage.text;
                sendReplyToUserCommand(update);
            }
        } else {
            sendTextMessageToUser(updateMessage.user.id, INVALID_COMMAND);
        }
    }
}

void Bot::executeUserCommand(const Update& update) {
    const Message& updateMessage = update.message;
    const int user_id = updateMessage.user.id;
    const QString& last_user_command = last_user_commands_[user_id];

    if (last_user_command == "/start") {
        sendCommandKeyboardToUser(user_id);
    }

    if (last_user_command == "/translate_text") {
        const QVector<QString>& languages = TextUtils::getFirstNWords(updateMessage.text, 2);
        const QString& text_to_translate = TextUtils::getTextAfterNthWord(updateMessage.text, 2);
        if (!text_to_translate.isEmpty() && languages.size() == 2) {
            const QPointer<Translater>& translater = new Translater(this, user_id);
            QString langFrom = languages.at(0).toLower();
            langFrom.resize(2);
            QString langTo = languages.at(1).toLower();
            langTo.resize(2);
            translater->translateText(text_to_translate, langFrom, langTo);
        } else {
            sendTextMessageToUser(user_id, toString(update) + INVALID_COMMAND);
        }
        last_user_commands_[user_id].clear();
    } else if (last_user_command == "/translate_file") {
        const QString& mime_type = updateMessage.document.mime_type;
        const QString& image_type = "image";
        auto it = std::search(mime_type.begin(), mime_type.end(), image_type.begin(), image_type.end());
        if (!updateMessage.document.isEmpty() && it == mime_type.end()) {
            QPointer<TelegramFileDownloader> file_downloader = new TelegramFileDownloader(this);
            file_downloader->downloadDocument(update);
        } else {
            sendTextMessageToUser(user_id, INVALID_TYPE_OF_TEXT_FILE);
        }
    } else if (last_user_command == "/recognize_photo" || last_user_command == "/translate_photo") {
        if (!updateMessage.photo_id.isEmpty()) {
            QPointer<TelegramFileDownloader> file_downloader = new TelegramFileDownloader(this);
            file_downloader->downloadPhoto(update);
        } else {
            sendTextMessageToUser(user_id, INVALID_TYPE_OF_PHOTO_FILE);
        }
    } else if (last_user_command == "/write_text_to_file") {
        const QString& text_message = updateMessage.text;
        const QString& text_uid = QString::number(user_id);
        if (!text_message.isEmpty()) {
            const QString& filename = text_uid + ".txt";
            TextUtils::writeToFile(filename, text_message);
            sendDocumentToUser(user_id, filename);
        } else {
            sendTextMessageToUser(user_id, "Invalid input data!");
        }
        last_user_commands_[user_id].clear();
    }
}

void Bot::sendReplyToUserCommand(const Update& update) {
    const int user_id = update.message.user.id;
    const QString& last_user_command = last_user_commands_[user_id];
    QString reply;
    if (last_user_command == "/start") {
        sendCommandKeyboardToUser(user_id);
        last_user_commands_[user_id].clear();
        return;
    } else if (last_user_command == "/translate_text") {
        reply =
            "Send text to the bot in format : <source_lang> <target_lang> "
            "<some "
            "text>.\nSource and target languages should be in 3-letter format. "
            "For example : English - eng, Ukrainian - ukr.\nAlso bot can "
            "detect "
            "source language. For that write to <souce_lang> auto.";
    } else if (last_user_command == "/translate_file") {
        reply =
            "Send text file to the bot with caption : <source_lang> "
            "<target_lang>.\nSource and target languages should be in 3-letter "
            "format."
            "For example : English - en, Ukrainian - ukr.\nAlso bot can detect "
            "source language. For that write to <souce_lang> auto.";
    } else if (last_user_command == "/recognize_photo") {
        reply =
            "Send photo to bot with caption : <source_lang>.\nSource language "
            "should be in 3-letter format."
            "For example : English - eng, Ukrainian - ukr.";
    } else if (last_user_command == "/write_text_to_file") {
        reply =
            "Send text message to bot in different language and bot will send "
            "file, which contains this text message";
    } else if (last_user_command == "/translate_photo") {
        reply =
            "Send photo to bot with caption : <source_lang> "
            "<target_lang>.\nSource "
            "and target languages should be in 3-letter format."
            "For example : English - eng, Ukrainian - ukr.";
    } else if (last_user_command == "/commands") {
        reply = "Valid commands : \n";
        for (auto command : VALID_COMMANDS) {
            reply += command + '\n';
        }
        last_user_commands_[user_id].clear();
    } else if (last_user_command == "/description") {
        reply = DESCRIPTION;
        last_user_commands_[user_id].clear();
    } else if (last_user_command == "/about") {
        reply = ABOUT;
        last_user_commands_[user_id].clear();
    } else if (last_user_command == "/supported_languages") {
        reply = "Supported languages for recognizing : \n";
        for (auto language : SUPPORTED_LANGUAGES) {
            reply += language + '\n';
        }
        last_user_commands_[user_id].clear();
    }
    sendTextMessageToUser(user_id, reply);
}

void Bot::sendCommandKeyboardToUser(int chat_id) {
    const QString& reply_markup = TelegramTypesFactory::buildJsonCommandKeyboardObject(KEYBOARD_BUTTONS);
    send_request_.setUrl(QUrl(TELEGRAM_API_URL + "bot" + BOT_TOKEN +
                              "/sendMessage?chat_id=" + QString::number(chat_id) +
                              "&text=Choose the command"
                              "&reply_markup=" +
                              reply_markup));
    send_access_manager_.get(send_request_);
}

void Bot::receiveTranslatedText(const QString& translated_text, int user_id) {
    if (translated_text.isEmpty()) {
        sendTextMessageToUser(user_id,
                              "Translation error. Something was wrong %F0%9F%98%93");
    } else {
        sendTextMessageToUser(user_id, translated_text);
        const QString& text_uid = QString::number(user_id);
        const QString& filename = text_uid + ".txt";
        TextUtils::writeToFile(filename, translated_text);
        sendDocumentToUser(user_id, filename);
    }
}

void Bot::receiveLocalFilePath(const QString& local_file_path, const Update& update) {
    int user_id = update.message.user.id;
    if (!local_file_path.isEmpty()) {
        const QString& last_user_command = last_user_commands_[user_id];
        const QString& caption = update.message.caption;
        if (last_user_command == "/translate_file") {
            QPointer<Translater> translater = new Translater(this, user_id);
            QString langFrom = TextUtils::getNthWord(caption, 1).toLower();
            QString langTo = TextUtils::getNthWord(caption, 2).toLower();
            langFrom.resize(2);
            langTo.resize(2);
            translater->translateFile(local_file_path, langFrom, langTo);
        } else if (last_user_command == "/recognize_photo") {
            QString langFrom = TextUtils::getNthWord(caption, 1).toLower();
            const QString& text = TesseractOCR::recognizeImage(local_file_path, langFrom);
            sendTextMessageToUser(user_id, text);
            const QString& text_uid = QString::number(user_id);
            const QString& filename = text_uid + ".txt";
            TextUtils::writeToFile(filename, text);
            sendDocumentToUser(user_id, filename);
        } else if (last_user_command == "/translate_photo") {
            QString langFrom = TextUtils::getNthWord(caption, 1).toLower();
            QString langTo = TextUtils::getNthWord(caption, 2).toLower();
            const QString& text = TesseractOCR::recognizeImage(local_file_path, langFrom);
            QPointer<Translater> translater = new Translater(this, user_id);
            langFrom.resize(2);
            langTo.resize(2);
            translater->translateText(text, langFrom, langTo);
        } else {
            sendTextMessageToUser(user_id, INVALID_COMMAND);
        }
        last_user_commands_[user_id].clear();
    } else {
        sendTextMessageToUser(user_id, "File receiving error! Try again.");
    }
}

void Bot::receiveSendingResult(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        std::cout << "Send Success" << std::endl;
    } else {
        std::cout << "Send Error" << std::endl;
        QString s(reply->readAll());
        std::cout << s.toStdString();
    }
}

void Bot::sendTextMessageToUser(int chat_id, const QString& message) {
    send_request_.setUrl(QUrl(TELEGRAM_API_URL + "bot" + BOT_TOKEN +
                              "/sendMessage?chat_id=" + QString::number(chat_id) + "&text=" + message));
    send_access_manager_.get(send_request_);
}

void Bot::sendDocumentToUser(int chat_id, const QString& filename) {
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/*"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"document\"; filename=\"" + filename + "\""));

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        imagePart.setBodyDevice(&file);
        multiPart->append(imagePart);
        send_request_.setUrl(
            QUrl(TELEGRAM_API_URL + "bot" + BOT_TOKEN + "/sendDocument?chat_id=" + QString::number(chat_id)));
        QNetworkReply* reply = send_access_manager_.post(send_request_, multiPart);
        reply->deleteLater();
    }

    delete multiPart;
}
