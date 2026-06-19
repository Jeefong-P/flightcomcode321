#include "FileLogger.h"
#include <cstring>
#include <cstdio>

FileLogger::FileLogger(SDCard* sd, const char* filename)
    : sdCard(sd), fileReady(false), filename(filename), wbytes(0) {

}

bool FileLogger::begin() {
    if (!sdCard->isInit()) {
        return false;
    }
    snprintf(filePath, sizeof(filePath), "%s/%s", sdCard->getDirName(), filename);
    FRESULT res = f_open(&file, filePath, FA_OPEN_ALWAYS | FA_WRITE);
    if (res == FR_OK) {
        f_lseek(&file, f_size(&file));  // Seek to end for append
        fileReady = true;
    }

    return fileReady;
}

void FileLogger::log(const char* format, ...) {
    if (!fileReady) return;

    char buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    UINT wbytes_;
    f_write(&file, buf, strlen(buf), &wbytes_);
    wbytes += wbytes_;
}

bool FileLogger::logStruct(const void* data, size_t size) {
    if (!fileReady || data == nullptr || size == 0) return false;

    UINT wbytes_ = 0;
    FRESULT res = f_write(&file, data, size, &wbytes_);
    wbytes += wbytes_;

    return (res == FR_OK && wbytes_ == size);
}


uint64_t FileLogger::sync() {
    if (fileReady) {
        f_sync(&file);
        total_wbytes += wbytes;
        uint64_t tmp = wbytes;
        wbytes = 0;
        return tmp;
    }else{
    	return 0;
    }
}
