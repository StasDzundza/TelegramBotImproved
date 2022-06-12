#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class PostService : public QObject {
    Q_OBJECT
   public:
    PostService(const QString& bot_token);
    void sendTextMessageToUser(int chat_id, const QString& message);
    void sendDocumentToUser(int chat_id, const QString& filename);
    void sendCommandKeyboardToUser(int chat_id);

   private slots:
    void receiveSendingResult(QNetworkReply* reply);

   private:
    QString bot_token_;
    QNetworkAccessManager send_access_manager_;
    QNetworkRequest send_request_;
};
