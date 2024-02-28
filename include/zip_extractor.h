#ifndef ZIPEXTRACTOR_H
#define ZIPEXTRACTOR_H

#include <zlib.h>

class ZipExtractor {
public:
    // Extract a zip file to a specified path
    bool ExtractZip(const std::string& zip_path, const std::string& extract_path);
}

#endif // ZIPEXTRACTOR_H
