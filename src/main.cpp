#include <QCoreApplication>

#include "bot.h"

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    Bot textRecognitionBot;
    textRecognitionBot.start();
    return a.exec();
}
