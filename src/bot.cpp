#include "bot.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QPointer>

#include "database.h"
#include "file_downloader.h"
#include "logger.h"
#include "pretty_printers.h"
#include "telegram_types_factory.h"
#include "tesseract_ocr.h"
#include "text_utils.h"
#include "translater.h"

// clazy:skip
namespace {
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
const QVector<QString> SUPPORTED_LANGUAGES{"English", "Ukrainian", "Russian"};
}  // namespace

Bot::Bot() : bot_token_(qgetenv("BOT_TOKEN")), post_service_(bot_token_) {
    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    update_request_.setSslConfiguration(sslConfiguration);
    connect(&update_access_manager_, &QNetworkAccessManager::finished, this, &Bot::receiveUpdates);
}

void Bot::start() {
    update_request_.setUrl(QUrl("https://api.telegram.org/bot" + bot_token_ + "/getUpdates"));
    update_access_manager_.get(update_request_);
}

QString Bot::getToken() const { return bot_token_; }

void Bot::receiveUpdates(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray& telegram_answer = reply->readAll();
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
        update_request_.setUrl(QUrl("https://api.telegram.org/bot" + bot_token_ +
                                    "/getUpdates?offset=" + QString::number(newest_update_id + 1)));
        update_access_manager_.get(update_request_);
    }
}

void Bot::processUpdate(const Update& update) {
    const Message& updateMessage = update.message;
    if (!updateMessage.isEmpty()) {
        const int user_id = updateMessage.user.id;
        const QString& last_user_command = Database::getInstance().getLastUserCommand(user_id);
        if (VALID_COMMANDS.contains(updateMessage.text) || !last_user_command.isEmpty()) {
            if (!last_user_command.isEmpty() && !VALID_COMMANDS.contains(updateMessage.text)) {
                Logger::logDebug(QString("[Bot] Executing last command for user %1").arg(QString::number(user_id)));
                executeUserCommand(update);
            } else {
                Logger::logDebug(QString("[Bot] Storing last command for user %1").arg(QString::number(user_id)));
                Database::getInstance().storeLastUserCommand(user_id, updateMessage.text);
                sendReplyToUserCommand(update);
            }
        } else {
            post_service_.sendTextMessageToUser(updateMessage.user.id, INVALID_COMMAND);
        }
    }
}

void Bot::executeUserCommand(const Update& update) {
    const Message& updateMessage = update.message;
    const int user_id = updateMessage.user.id;
    const QString& last_user_command = Database::getInstance().getLastUserCommand(user_id);

    if (last_user_command == "/start") {
        post_service_.sendCommandKeyboardToUser(user_id);
    } else if (last_user_command == "/translate_text") {
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
            post_service_.sendTextMessageToUser(user_id, toString(update) + INVALID_COMMAND);
        }
        Database::getInstance().clearLastUserCommand(user_id);
    } else if (last_user_command == "/translate_file") {
        const QString& mime_type = updateMessage.document.mime_type;
        const QString& image_type = "image";
        auto it = std::search(mime_type.begin(), mime_type.end(), image_type.begin(), image_type.end());
        if (!updateMessage.document.isEmpty() && it == mime_type.end()) {
            QPointer<TelegramFileDownloader> file_downloader = new TelegramFileDownloader(this);
            file_downloader->downloadDocument(update);
        } else {
            post_service_.sendTextMessageToUser(user_id, INVALID_TYPE_OF_TEXT_FILE);
        }
    } else if (last_user_command == "/recognize_photo" || last_user_command == "/translate_photo") {
        if (!updateMessage.photo_id.isEmpty()) {
            QPointer<TelegramFileDownloader> file_downloader = new TelegramFileDownloader(this);
            file_downloader->downloadPhoto(update);
        } else {
            post_service_.sendTextMessageToUser(user_id, INVALID_TYPE_OF_PHOTO_FILE);
        }
    } else if (last_user_command == "/write_text_to_file") {
        const QString& text_message = updateMessage.text;
        const QString& text_uid = QString::number(user_id);
        if (!text_message.isEmpty()) {
            const QString& filename = text_uid + ".txt";
            TextUtils::writeToFile(filename, text_message);
            post_service_.sendDocumentToUser(user_id, filename);
        } else {
            post_service_.sendTextMessageToUser(user_id, "Invalid input data!");
        }
        Database::getInstance().clearLastUserCommand(user_id);
    }
}

void Bot::sendReplyToUserCommand(const Update& update) {
    const int user_id = update.message.user.id;
    const QString& last_user_command = Database::getInstance().getLastUserCommand(user_id);
    Logger::logDebug(
        QString("[Bot] last_user_command for user %1 is %2").arg(QString::number(user_id), last_user_command));
    QString reply;
    if (last_user_command == "/start") {
        post_service_.sendCommandKeyboardToUser(user_id);
        Database::getInstance().clearLastUserCommand(user_id);
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
        Database::getInstance().clearLastUserCommand(user_id);
    } else if (last_user_command == "/description") {
        reply = DESCRIPTION;
        Database::getInstance().clearLastUserCommand(user_id);
    } else if (last_user_command == "/about") {
        reply = ABOUT;
        Database::getInstance().clearLastUserCommand(user_id);
    } else if (last_user_command == "/supported_languages") {
        reply = "Supported languages for recognizing : \n";
        for (auto language : SUPPORTED_LANGUAGES) {
            reply += language + '\n';
        }
        Database::getInstance().clearLastUserCommand(user_id);
    }
    post_service_.sendTextMessageToUser(user_id, reply);
}

void Bot::receiveTranslatedText(const QString& translated_text, int user_id) {
    Logger::logDebug(QString("[Bot] Received translated text: %1").arg(translated_text));
    if (translated_text.isEmpty()) {
        post_service_.sendTextMessageToUser(user_id, "Translation error. Something was wrong %F0%9F%98%93");
    } else {
        post_service_.sendTextMessageToUser(user_id, translated_text);
        const QString& text_uid = QString::number(user_id);
        const QString& filename = QString("%1.txt").arg(text_uid);
        TextUtils::writeToFile(filename, translated_text);
        post_service_.sendDocumentToUser(user_id, filename);
    }
}

void Bot::receiveLocalFilePath(const QString& local_file_path, const Update& update) {
    Logger::logDebug(QString("[Bot] Received local file path: %1").arg(local_file_path));
    int user_id = update.message.user.id;
    if (!local_file_path.isEmpty()) {
        const QString& last_user_command = Database::getInstance().getLastUserCommand(user_id);
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
            post_service_.sendTextMessageToUser(user_id, text);
            const QString& text_uid = QString::number(user_id);
            const QString& filename = text_uid + ".txt";
            TextUtils::writeToFile(filename, text);
            post_service_.sendDocumentToUser(user_id, filename);
        } else if (last_user_command == "/translate_photo") {
            QString langFrom = TextUtils::getNthWord(caption, 1).toLower();
            QString langTo = TextUtils::getNthWord(caption, 2).toLower();
            const QString& text = TesseractOCR::recognizeImage(local_file_path, langFrom);
            QPointer<Translater> translater = new Translater(this, user_id);
            langFrom.resize(2);
            langTo.resize(2);
            translater->translateText(text, langFrom, langTo);
        } else {
            post_service_.sendTextMessageToUser(user_id, INVALID_COMMAND);
        }
        Database::getInstance().clearLastUserCommand(user_id);
    } else {
        post_service_.sendTextMessageToUser(user_id, "File receiving error! Try again.");
    }
}
