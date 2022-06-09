#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

#include "telegram_types.h"

class Bot;

class TelegramFileDownloader : public QObject {
    Q_OBJECT
   public:
    explicit TelegramFileDownloader(const Bot* bot);

    void downloadDocument(const Update& update);
    void downloadPhoto(const Update& update);

   signals:
    void sendLocalFilePath(const QString&, const Update&);

   private slots:
    void receiveFileData(QNetworkReply*);
    void receiveFilePath(QNetworkReply*);

   private:
    QString bot_token_;
    Update update_;

    QNetworkAccessManager file_path_access_manager_, file_data_access_manager_;
    QNetworkRequest file_path_request_, file_data_request_;

    void getFilePath(const QString& file_id);
};
