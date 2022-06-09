#pragma once

#include <QString>

#include "telegram_types.h"

QString toString(const Message& message);
QString toString(const Update& update);
QString toString(const User& user);
