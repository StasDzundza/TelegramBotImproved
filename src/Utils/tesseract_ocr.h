#pragma once

// #include <baseapi.h>
// #include <allheaders.h>
#include <QString>

class TesseractOCR {
   public:
    static QString recognizeImage(const QString& path_to_image, const QString& source_lang);

   private:
    static QString tesseractPreprocess(const QString& source_file);
    static QString tesseractOcr(const QString& preprocessed_file, const QString& source_lang);
};
