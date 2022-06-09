#pragma once

#include <QString>
#include <QVector>

class TextUtils {
   public:
    static QVector<QString> splitTextByWords(const QString& text);
    static QString getTextAfterNthWord(const QString& text, int word_num);
    static QVector<QString> getFirstNWords(const QString& text, int n);
    static QString getNthWord(const QString& text, int n);
    static QString readFile(const QString& file_path);
    static void writeToFile(const QString& filepath, const QString& text);
};
