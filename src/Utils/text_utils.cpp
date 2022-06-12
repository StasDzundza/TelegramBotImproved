#include "text_utils.h"

#include <QFile>
#include <QTextStream>
#include <sstream>

QVector<QString> TextUtils::splitTextByWords(const QString& text) {
    QVector<QString> words;
    const std::string& command = text.toStdString();
    // Used to split string around spaces.
    std::istringstream ss(command);
    // Traverse through all words
    do {
        std::string word;
        ss >> word;
        if (!word.empty()) {
            words << QString::fromStdString(word);
        }
    } while (ss);
    return words;
}

QString TextUtils::getTextAfterNthWord(const QString& text, int word_num) {
    QString text_after_nth_word;
    int num_of_chars_before_word = 0;
    const std::string& command = text.toStdString();
    std::istringstream ss(command);
    int tmp_num = word_num;
    while (tmp_num-- && ss) {
        std::string word;
        ss >> word;
        if (!word.empty()) {
            num_of_chars_before_word += word.size();
        }
    }
    num_of_chars_before_word += word_num;
    if (text.size() > num_of_chars_before_word) {
        std::copy(text.begin() + num_of_chars_before_word, text.end(), std::back_inserter(text_after_nth_word));
    }
    return text_after_nth_word;
}

QVector<QString> TextUtils::getFirstNWords(const QString& text, int n) {
    QVector<QString> words;
    const std::string& command = text.toStdString();
    std::istringstream ss(command);
    while (n-- && ss) {
        std::string word;
        ss >> word;
        if (!word.empty()) {
            words << QString::fromStdString(word);
        }
    }
    return words;
}

QString TextUtils::getNthWord(const QString& text, int n) {
    QString nth_word;
    const std::string& command = text.toStdString();
    std::istringstream ss(command);
    std::string word;
    while (n-- && ss) {
        ss >> word;
    }
    nth_word = QString::fromStdString(word);
    return nth_word;
}

QString TextUtils::readFile(const QString& file_path) {
    QFile localFile(file_path);
    if (localFile.open(QIODevice::ReadOnly)) {
        const QString& file_data = QString::fromStdString(localFile.readAll().toStdString());
        localFile.close();
        return file_data;
    }
    return "";
}

void TextUtils::writeToFile(const QString& filepath, const QString& text) {
    QFile qFile(filepath);

    if (qFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream out(&qFile);
        out << text;
        qFile.close();
    }
}
