#ifndef SDCARD_FILELOGGER_H_
#define SDCARD_FILELOGGER_H_

#include "SDCard.h"
#include "fatfs.h"
#include <cstdarg>
#include <cstddef>  // for size_t

class FileLogger {
public:
    FileLogger(SDCard* sd, const char* filename);
    bool begin();

    // Log printf-style text
    void log(const char* format, ...);

    // Log raw binary struct
    bool logStruct(const void* data, size_t size);

    uint64_t sync();

private:
    SDCard* sdCard;
    FIL file;
    bool fileReady;
    const char *filename;
    char filePath[64];
    uint64_t total_wbytes, wbytes;
};

#endif /* SDCARD_FILELOGGER_H_ */
