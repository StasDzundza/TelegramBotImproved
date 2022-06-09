#pragma once

#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSet>

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
    void receiveSendingResult(QNetworkReply* reply);

   private:
    QString BOT_TOKEN;
    QMap<int, QString> last_user_commands_;
    QNetworkAccessManager update_access_manager_, send_access_manager_;
    QNetworkRequest update_request_, send_request_;

    void processUpdate(const Update& update);
    void executeUserCommand(const Update& update);
    void sendTextMessageToUser(int chat_id, const QString& message);
    void sendDocumentToUser(int chat_id, const QString& filename);
    void sendReplyToUserCommand(const Update& update);
    void sendCommandKeyboardToUser(int chat_id);
};
