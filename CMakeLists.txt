cmake_minimum_required(VERSION 3.14)

project(TelegramBotImproved LANGUAGES CXX)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(CMAKE_OSX_ARCHITECTURES x86_64)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Network REQUIRED)

set (sources
    src/main.cpp
    src/bot.cpp
    # Telegram Types
    src/TelegramTypes/telegram_types_factory.cpp
    # Utils
    src/Utils/file_downloader.cpp
    src/Utils/pretty_printers.cpp
    src/Utils/tesseract_ocr.cpp
    src/Utils/text_utils.cpp
    src/Utils/translater.cpp
)

set (headers
    src/bot.h
    # Telegram Types
    src/TelegramTypes/telegram_types.h
    src/TelegramTypes/telegram_types_factory.h
    # Utils
    src/Utils/file_downloader.h
    src/Utils/pretty_printers.h
    src/Utils/tesseract_ocr.h
    src/Utils/text_utils.h
    src/Utils/translater.h
)

add_executable(TelegramBotImproved ${sources} ${headers})

target_include_directories(TelegramBotImproved PRIVATE src src/TelegramTypes src/Utils)
target_link_libraries(TelegramBotImproved PRIVATE Qt5::Core Qt5::Gui Qt5::Network)
target_compile_options(TelegramBotImproved PRIVATE -Werror)
