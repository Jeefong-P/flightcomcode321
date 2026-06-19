#include "SDCard.h"


SDCard::SDCard() : init(false) {
    // Constructor
}

SDCardError SDCard::begin() {
    MX_FATFS_Init();

    if (f_mount(&SDFatFS, (TCHAR const*) SDPath, 0) == FR_OK) {
        return createFlightLogDirectory();
    } else {
    	return F_MOUNT_FAILED;
    }
}

bool SDCard::isInit() {
	return init;
}

char* SDCard::getDirName(){
	return dir_name;
}

SDCardError SDCard::createFlightLogDirectory(){
    char file_path[32];
    FRESULT res;
    DIR dir;

    for(uint16_t flight_num = 1; flight_num <= 999; ++flight_num){
        sprintf(dir_name, "FlightLog_%03d", flight_num);

        res = f_opendir(&dir, dir_name);

        if (res == FR_NO_PATH || res == FR_NO_FILE) {
            // Directory does not exist, safe to create
            res = f_mkdir(dir_name);
            if(res == FR_OK){
                // Create readme.txt
                sprintf(file_path, "%s/readme.txt", dir_name);
                if (f_open(&SDFile, file_path, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
                    UINT wbytes;
                    char msg[] = "\xEF\xBB\xBFNever gonna give you up.\r\nNever gonna let you down.\r\n";
                    if (f_write(&SDFile, msg, sizeof(msg), &wbytes) == FR_OK) {
                        f_close(&SDFile);

                        init = 1;
                        return SD_SUCCESS;
                    } else {
                        f_close(&SDFile);
                        return F_WRITE_FAILED;
                    }
                } else {
                    f_closedir(&dir);
                    return F_WRITE_FAILED;
                }
            } else {
                // mkdir failed
                return F_MKDIR_FAILED;
            }
        } else if (res == FR_OK) {
            // Directory exists, increment and try again
            f_closedir(&dir);
        } else {
            // Some other error
            return UNKNOWN;
        }
    }

    // If we reach here, we've hit the limit (99 directories)
    // Delete FlightLog_001 and recreate it
    sprintf(dir_name, "FlightLog_001");

    // Delete the directory and its contents recursively
    // Note: FatFs doesn't have recursive delete, so we need to delete files first
    res = f_opendir(&dir, dir_name);
    if(res == FR_OK) {
        FILINFO fno;
        while(f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
            sprintf(file_path, "%s/%s", dir_name, fno.fname);
            f_unlink(file_path);  // Delete file
        }
        f_closedir(&dir);
        f_unlink(dir_name);  // Delete empty directory
    }

    // Now create FlightLog_099
    res = f_mkdir(dir_name);
    if(res == FR_OK){
        // Create readme.txt
        sprintf(file_path, "%s/readme.txt", dir_name);
        if (f_open(&SDFile, file_path, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
            UINT wbytes;
            char msg[] = "\xEF\xBB\xBFNever gonna give you up.\r\nNever gonna let you down.\r\n";
            if (f_write(&SDFile, msg, sizeof(msg), &wbytes) == FR_OK) {
                f_close(&SDFile);

                init = 1;
                return SD_SUCCESS;
            } else {
                f_close(&SDFile);
                return F_WRITE_FAILED;
            }
        } else {
            return F_WRITE_FAILED;
        }
    } else {
        // mkdir failed
        return F_MKDIR_FAILED;
    }
}
