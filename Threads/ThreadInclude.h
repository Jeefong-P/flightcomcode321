#ifndef THREAD_INCLUDE_H
#define THREAD_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

// Sensor thread functions
void ADXL375Thread(void *argument);        ///< High-G accelerometer thread
void IMUThread(void *argument);            ///< ICM-45686 IMU sensor thread
void MS5607Thread(void *argument);         ///< Barometric sensor thread
void GNSSThread(void *argument);           ///< GPS/GNSS thread

// Communication and storage threads
void LoRaThread(void *argument);           ///< LoRa stub (removed)
void SDCardThread(void *argument);         ///< SD card logging thread
void UARTThread(void *argument);           ///< UART telemetry to ESP32

// System management threads
void FlightLogicThread(void *argument);    ///< Flight state logic thread
void BuzzerThread(void *argument);         ///< Audio feedback thread
void GreenLedThread(void *argument);       ///< Status LED thread

#ifdef __cplusplus
}
#endif

#endif /* THREAD_INCLUDE_H */
