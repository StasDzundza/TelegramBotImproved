#include "file_downloader.h"

#include <QFile>
#include <QJsonDocument>

#include "bot.h"
#include "telegram_types_factory.h"

TelegramFileDownloader::TelegramFileDownloader(const Bot* bot) : QObject(nullptr), bot_token_(bot->getToken()) {
    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    file_path_request_.setSslConfiguration(sslConfiguration);
    file_data_request_.setSslConfiguration(sslConfiguration);

    connect(&file_path_access_manager_, &QNetworkAccessManager::finished, this,
            &TelegramFileDownloader::receiveFilePath);
    connect(&file_data_access_manager_, &QNetworkAccessManager::finished, this,
            &TelegramFileDownloader::receiveFileData);
    connect(this, &TelegramFileDownloader::sendLocalFilePath, bot, &Bot::receiveLocalFilePath);
}

void TelegramFileDownloader::downloadDocument(const Update& update) {
    update_ = update;
    file_path_request_.setUrl(QUrl("https://api.telegram.org/bot" + bot_token_ +
                                   "/getFile?file_id=" + update.message.document.file_id));
    file_path_access_manager_.get(file_path_request_);
}

void TelegramFileDownloader::downloadPhoto(const Update& update) {
    update_ = update;
    file_path_request_.setUrl(
        QUrl("https://api.telegram.org/bot" + bot_token_ + "/getFile?file_id=" + update.message.photo_id));
    file_path_access_manager_.get(file_path_request_);
}

void TelegramFileDownloader::receiveFilePath(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray& telegram_answer = reply->readAll();
        const QJsonDocument& json_document = QJsonDocument::fromJson(telegram_answer);
        const QJsonObject& rootObject = json_document.object();
        File file = TelegramTypesFactory::createFile(rootObject);
        file_data_request_.setUrl("https://api.telegram.org/file/bot" + bot_token_ + '/' + file.file_path);
        file_data_access_manager_.get(file_data_request_);
    }
}

void TelegramFileDownloader::receiveFileData(QNetworkReply* reply) {
    const QString& file_path = QString::number(update_.message.user.id);
    QFile localFile(file_path);

    if (localFile.open(QIODevice::WriteOnly)) {
        localFile.write(reply->readAll());
        localFile.close();
        emit sendLocalFilePath(file_path, update_);
    }
}
