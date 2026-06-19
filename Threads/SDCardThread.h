/**
 * @file SDCardThread.h
 * @brief SD card data logging management thread
 * @author Korn
 * @date June 2025
 *
 * This file manages all data logging to SD card. Multiple log files are
 * maintained for different sensors and systems, with periodic synchronization
 * to ensure data integrity during flight.
 */

#ifndef SDCARDTHREAD_H_
#define SDCARDTHREAD_H_

#include "SDCard.h"
#include "FileLogger.h"
#include "FlightLogicThread.h"
#include "GlobalInclude.h"

// External SD card handle
extern SD_HandleTypeDef hsd2;

// SD card and file logger instances
SDCard sdCard;
FileLogger ms5607Logger(&sdCard, "ms5607.log");           // Barometric sensor data
FileLogger icmAccLogger(&sdCard, "icm_acc.log");      // IMU accelerometer data
FileLogger icmGyroLogger(&sdCard, "icm_gyro.log");    // IMU gyroscope data
FileLogger adxl375Logger(&sdCard, "adxl375.log");         // High-G accelerometer data
FileLogger flightLogicLogger(&sdCard, "flightLogic.log"); // Flight state data
FileLogger batLevelLogger(&sdCard, "batLevel.log");

// Performance monitoring
uint64_t total_wbytes = 0;      // Total bytes written to SD card
double avg_wbytes = 0.0;        // Average write rate (bytes/second)

/**
 * @brief Initialize SD card SDMMC2 interface
 * @note Configures SDMMC2 peripheral for 4-bit wide bus operation
 */
//void MX_SDMMC2_SD_Init(void) {
//    // Configure SDMMC2 peripheral
//    hsd2.Instance = SDMMC2;
//    hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
//    hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
//    hsd2.Init.BusWide = SDMMC_BUS_WIDE_4B;                    // 4-bit bus for higher speed
//    hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
//    hsd2.Init.ClockDiv = 80;                                  // Clock divider for stable operation
//
//
//    // Initialize SD card interface
//    HAL_StatusTypeDef ret = HAL_SD_Init(&hsd2);
//    if (ret != HAL_OK) {
//        // SD card hardware initialization failed - enter safe mode
//        while (1) {
//            osDelay(1000);
//        }
//    }
//}

/**
 * @brief SD card data logging thread
 * @param argument Thread parameter (unused)
 *
 * Manages all file logging operations including initialization of log files
 * and periodic synchronization to ensure data is written to SD card.
 */
void SDCardThread(void *argument) {
#ifdef DISABLE_SD_CARD
    // SD card disabled in build configuration
    while (1) {
        osDelay(1000);
    }
#endif

    // Initialize SD card hardware
//    MX_SDMMC2_SD_Init();
    osDelay(100);  // Allow SD card to stabilize

    // Initialize SD card file system
    SDCardError err = sdCard.begin();

    if (sdCard.isInit()) {
        // SD card successfully initialized - open all log files
        if (!ms5607Logger.begin()) {
            // Failed to create barometric sensor log file
        }
        if (!icmAccLogger.begin()) {
            // Failed to create IMU accelerometer log file
        }
        if (!icmGyroLogger.begin()) {
            // Failed to create IMU gyroscope log file
        }
        if (!adxl375Logger.begin()) {
            // Failed to create high-G accelerometer log file
        }
        if (!flightLogicLogger.begin()) {
            // Failed to create flight logic log file
        }
        batLevelLogger.begin();

        // Signal SD card system is ready
        initStatus.SDCardReady = 1;
    } else {
        // SD card initialization failed - enter safe mode
        while (1) {
            osDelay(1000);
        }
    }

    // Main logging synchronization loop
    for (;;) {
        // Periodically sync each log file to ensure data integrity
        // Small delays between operations prevent SD card bus conflicts

        total_wbytes += ms5607Logger.sync();
        osDelay(10);

        total_wbytes += icmAccLogger.sync();
        osDelay(10);

        total_wbytes += icmGyroLogger.sync();
        osDelay(10);

        total_wbytes += adxl375Logger.sync();
        osDelay(10);

        total_wbytes += flightLogicLogger.sync();
        osDelay(10);

        total_wbytes += batLevelLogger.sync();
        osDelay(10);

        // Calculate average write performance
        avg_wbytes = (double)total_wbytes / (HAL_GetTick()/1000.0);

        // Sync operation interval
        osDelay(100);
    }
}

#endif /* SDCARDTHREAD_H_ */
