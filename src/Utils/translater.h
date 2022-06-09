#pragma once

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

class Bot;

class Translater : public QObject {
    Q_OBJECT
   public:
    explicit Translater(const Bot* bot, int user_id);

    void translateText(const QString& text, const QString& langFrom, const QString& langTo);
    void translateFile(const QString& file_path, const QString& langFrom, const QString& langTo);

   signals:
    void sendTranslatedText(const QString& translated_text, int user_id);

   private slots:
    void receiveTranslate(QNetworkReply* translate_reply);

   private:
    int user_id_{-1};
    QNetworkAccessManager translate_access_manager_;
    QNetworkRequest translate_request_;

    QString parseTranslatedTextFromXml(const QByteArray& translate_result);
};
