#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "post_service.h"
#include "telegram_types.h"

class Bot : public QObject {
    Q_OBJECT
   public:
    Bot();
    void start();
    QString getToken() const;
    friend class TelegramFileDownloader;

   public slots:
    void receiveTranslatedText(const QString& translated_text, int user_id);
    void receiveLocalFilePath(const QString& local_file_path, const Update& update);

   private slots:
    void receiveUpdates(QNetworkReply* reply);

   private:
    QString bot_token_;
    QNetworkAccessManager update_access_manager_;
    QNetworkRequest update_request_;
    PostService post_service_;

    void processUpdate(const Update& update);
    void executeUserCommand(const Update& update);
    void sendReplyToUserCommand(const Update& update);
};
