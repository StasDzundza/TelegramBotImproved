#include "post_service.h"

#include <QFile>
#include <QHttpMultiPart>

#include "logger.h"
#include "telegram_types_factory.h"

// clazy:skip
namespace {
const QVector<QString> KEYBOARD_BUTTONS{"/translate_text", "/recognize_photo",    "/translate_photo",
                                        "/translate_file", "/write_text_to_file", "/supported_languages",
                                        "/commands",       "/description",        "/about"};
}  // namespace

PostService::PostService(const QString& bot_token) : bot_token_(bot_token) {
    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    send_request_.setSslConfiguration(sslConfiguration);
    connect(&send_access_manager_, &QNetworkAccessManager::finished, this, &PostService::receiveSendingResult);
}

void PostService::sendCommandKeyboardToUser(int chat_id) {
    Logger::logDebug(QString("[Post Service] Command keyboard was sent to user %1").arg(QString::number(chat_id)));
    const QString& reply_markup = TelegramTypesFactory::buildJsonCommandKeyboardObject(KEYBOARD_BUTTONS);
    send_request_.setUrl(QUrl("https://api.telegram.org/bot" + bot_token_ +
                              "/sendMessage?chat_id=" + QString::number(chat_id) +
                              "&text=Choose the command"
                              "&reply_markup=" +
                              reply_markup));
    send_access_manager_.get(send_request_);
}

void PostService::sendTextMessageToUser(int chat_id, const QString& message) {
    Logger::logDebug(QString("[Post Service] Text message: %1 was sent to user %2").arg(message, QString::number(chat_id)));
    send_request_.setUrl(QUrl("https://api.telegram.org/bot" + bot_token_ +
                              "/sendMessage?chat_id=" + QString::number(chat_id) + "&text=" + message));
    send_access_manager_.get(send_request_);
}

void PostService::sendDocumentToUser(int chat_id, const QString& filename) {
    Logger::logDebug(QString("[Post Service] File: %1 was sent to user %2").arg(filename, QString::number(chat_id)));
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
            QUrl("https://api.telegram.org/bot" + bot_token_ + "/sendDocument?chat_id=" + QString::number(chat_id)));
        QNetworkReply* reply = send_access_manager_.post(send_request_, multiPart);
        reply->deleteLater();
    }

    delete multiPart;
}

void PostService::receiveSendingResult(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        Logger::logInfo("Sending was successfull");
    } else {
        Logger::logCritical("Sending error: " + reply->readAll());
    }
}
