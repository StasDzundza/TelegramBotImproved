#pragma once

#include <QString>

class Logger {
   public:
    enum Severity { Critical, Info, Debug };

    static void logMessage(const QString& message, Severity severity);
    static void logCritical(const QString& message);
    static void logInfo(const QString& message);
    static void logDebug(const QString& message);
};
