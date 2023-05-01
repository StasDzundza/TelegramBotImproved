#include "logger.h"

#include <QDateTime>
#include <QDebug>
#include <QTextStream>

#include "text_utils.h"

// clazy:skip
namespace {
const QString LOG_FILENAME = "bot.log";
};

void Logger::logCritical(const QString &message) { logMessage(message, Severity::Critical); }

void Logger::logInfo(const QString &message) { logMessage(message, Severity::Info); }

void Logger::logDebug(const QString &message) { logMessage(message, Severity::Debug); }

void Logger::logMessage(const QString &message, Severity severity) {
    QString prefix;
    switch (severity) {
        case Severity::Critical:
            prefix = "[CRITICAL] ";
        case Severity::Info:
            prefix = "[INFO] ";
        case Severity::Debug:
            prefix = "[DEBUG] ";
    }
    const QString &timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString log_message;
    QTextStream stream(&log_message);
    stream << "[" << timestamp << "]" << prefix << message << "\n";
    TextUtils::writeToFile(LOG_FILENAME, log_message);
    if (severity == Severity::Debug) {
        qDebug() << log_message;
    }
}
