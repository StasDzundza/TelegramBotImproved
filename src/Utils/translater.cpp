#include "translater.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QXmlStreamReader>

#include "bot.h"
#include "text_utils.h"

// clazy:skip
namespace {
const QString x_rapid_host = "microsoft-azure-translation-v1.p.rapidapi.com";
const QString x_rapid_key = "37df3f18b8msh4672e6c02bb8340p1caad0jsn60400187e4e1";
const QString translator_url = "https://microsoft-azure-translation-v1.p.rapidapi.com/translate";
const QString accept = "application/json";
}  // namespace

Translater::Translater(const Bot* bot, int user_id) : QObject(nullptr), user_id_(user_id) {
    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    translate_request_.setSslConfiguration(sslConfiguration);
    translate_request_.setRawHeader(QByteArray("x-rapidapi-host"), QByteArray(x_rapid_host.toStdString().c_str()));
    translate_request_.setRawHeader(QByteArray("x-rapidapi-key"), QByteArray(x_rapid_key.toStdString().c_str()));
    translate_request_.setRawHeader(QByteArray("accept"), QByteArray(accept.toStdString().c_str()));

    connect(&translate_access_manager_, &QNetworkAccessManager::finished, this, &Translater::receiveTranslate);
    connect(this, &Translater::sendTranslatedText, bot, &Bot::receiveTranslatedText);
}

void Translater::translateText(const QString& text, const QString& langFrom, const QString& langTo) {
    const QString& requestUrl = translator_url + "?from=" + langFrom + "&to=" + langTo + "&text=" + text;
    translate_request_.setUrl(QUrl(requestUrl));
    translate_access_manager_.get(translate_request_);
}

void Translater::translateFile(const QString& file_path, const QString& langFrom, const QString& langTo) {
    const QString& text = TextUtils::readFile(file_path);
    translateText(text, langFrom, langTo);
}

void Translater::receiveTranslate(QNetworkReply* translate_reply) {
    if (translate_reply->error() == QNetworkReply::NoError) {
        const QByteArray& translater_answer = translate_reply->readAll();
        const QString& translated_text = parseTranslatedTextFromXml(translater_answer);
        qDebug() << translated_text;
        emit sendTranslatedText(translated_text.toUtf8(), user_id_);
    } else {
        emit sendTranslatedText("", user_id_);
        const QByteArray& translater_answer = translate_reply->readAll();
        qDebug() << translater_answer;
    }
}

QString Translater::parseTranslatedTextFromXml(const QByteArray& translate_result) {
    QXmlStreamReader reader(translate_result);
    while (!reader.atEnd() && !reader.hasError()) {
        if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == "string") {
            return reader.readElementText();
        }
    }
    return "";
}
