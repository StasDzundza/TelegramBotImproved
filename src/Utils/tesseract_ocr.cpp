#include "tesseract_ocr.h"

#include <QCoreApplication>
#include <fstream>

QString TesseractOCR::recognizeImage(const QString &path_to_image, const QString &source_lang) {
    std::ifstream infile(path_to_image.toStdString());
    if (infile.good()) {
        // preprocess to convert to black white book-like text
        QString preprocessed_file = tesseractPreprocess(path_to_image);
        return tesseractOcr(preprocessed_file, source_lang);
    }
    return "";
}

QString TesseractOCR::tesseractPreprocess(const QString &source_file) {
    //    char tempPath[128];
    //    GetTempPathA(128, tempPath);
    //    strcat_s(tempPath, "preprocess_ocr.bmp");

    //    char preprocessed_file[MAX_PATH];
    //    strcpy_s(preprocessed_file, tempPath);

    //    BOOL perform_negate = TRUE;
    //    l_float32 dark_bg_threshold = 0.5f; // From 0.0 to 1.0, with 0 being
    //    all white and 1 being all black

    //    int perform_scale = 1;
    //    l_float32 scale_factor = 3.5f;

    //    int perform_unsharp_mask = 1;
    //    l_int32 usm_halfwidth = 5;
    //    l_float32 usm_fract = 2.5f;

    //    int perform_otsu_binarize = 1;
    //    l_int32 otsu_sx = 2000;
    //    l_int32 otsu_sy = 2000;
    //    l_int32 otsu_smoothx = 0;
    //    l_int32 otsu_smoothy = 0;
    //    l_float32 otsu_scorefract = 0.0f;

    //    l_int32 status = 1;
    //    l_float32 border_avg = 0.0f;
    //    PIX *pixs = nullptr;

    //    // Read in source image
    //    pixs = pixRead(source_file.toStdString().c_str());

    //    // Convert to grayscale
    //    pixs = pixConvertRGBToGray(pixs, 0.0f, 0.0f, 0.0f);

    //    if (perform_negate) {
    //        PIX *otsu_pixs = nullptr;

    //        status = pixOtsuAdaptiveThreshold(pixs, otsu_sx, otsu_sy,
    //        otsu_smoothx, otsu_smoothy, otsu_scorefract, NULL, &otsu_pixs);

    //        // Get the average intensity of the border pixels,
    //        // with average of 0.0 being completely white and 1.0 being
    //        completely black. border_avg = pixAverageOnLine(otsu_pixs, 0, 0,
    //        otsu_pixs->w - 1, 0, 1);                               // Top
    //        border_avg += pixAverageOnLine(otsu_pixs, 0, otsu_pixs->h - 1,
    //        otsu_pixs->w - 1, otsu_pixs->h - 1, 1); // Bottom border_avg +=
    //        pixAverageOnLine(otsu_pixs, 0, 0, 0, otsu_pixs->h - 1, 1); // Left
    //        border_avg += pixAverageOnLine(otsu_pixs, otsu_pixs->w - 1, 0,
    //        otsu_pixs->w - 1, otsu_pixs->h - 1, 1); // Right border_avg
    //        /= 4.0f;

    //        pixDestroy(&otsu_pixs);

    //        // If background is dark
    //        if (border_avg > dark_bg_threshold) {
    //            // Negate image
    //            pixInvert(pixs, pixs);

    //        }
    //    }

    //    if (perform_scale) {
    //        // Scale the image (linear interpolation)
    //        pixs = pixScaleGrayLI(pixs, scale_factor, scale_factor);
    //    }

    //    if (perform_unsharp_mask) {
    //        // Apply unsharp mask
    //        pixs = pixUnsharpMaskingGray(pixs, usm_halfwidth, usm_fract);
    //    }

    //    if (perform_otsu_binarize) {
    //        // Binarize
    //        status = pixOtsuAdaptiveThreshold(pixs, otsu_sx, otsu_sy,
    //        otsu_smoothx, otsu_smoothy, otsu_scorefract, NULL, &pixs);
    //    }
    //    // Write the image to file
    //    status = pixWriteImpliedFormat(preprocessed_file, pixs, 0, 0);

    //    QString out(preprocessed_file);

    //    return out;
    return "";
}

QString TesseractOCR::tesseractOcr(const QString &preprocessed_file, const QString &source_lang) {
    //    Pix *image = pixRead(preprocessed_file.toStdString().c_str());
    //    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

    //    QString app_name = QCoreApplication::applicationName();
    //    QString tesseract_train_data = "..\\" + app_name + "\\ TesseractOCR";
    //    api->Init(tesseract_train_data.toStdString().c_str(),
    //    source_lang.toStdString().c_str());
    //    api->SetPageSegMode(tesseract::PSM_AUTO_OSD); // PSM_SINGLE_BLOCK

    //    api->SetImage(image);

    //    return api->GetUTF8Text();
    return "";
}
