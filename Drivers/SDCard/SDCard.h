#ifndef SDCARD_H
#define SDCARD_H

#include "fatfs.h"
#include "cmsis_os.h"
#include "main.h" // for Error_Handler
#include <stdio.h>

enum SDCardError{
	SD_SUCCESS = 0,
	F_MOUNT_FAILED,
	F_WRITE_FAILED,
	F_OPEN_FAILED,
	F_MKDIR_FAILED,
	TOO_MANY_FLIGHT_LOGS,
	UNKNOWN
};

class SDCard {
public:
    SDCard();
    SDCardError begin();
    bool isInit();
    char* getDirName();

private:
    SDCardError createFlightLogDirectory();

    char dir_name[14];
    bool init;
};


#endif // SDCARD_H
