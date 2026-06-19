#ifndef UARTTHREAD_H_
#define UARTTHREAD_H_

#include "cmsis_os2.h"
#include "stm32u5xx_hal.h"
#include "GlobalInclude.h"

extern UART_HandleTypeDef huart4;
extern MS5607 ms5607;
extern ICM45686_Handle icm;
extern float altitude;
extern float acceleration;
extern FlightState flightState;

#define CALLSIGN "KO6IRZ"

/**
 * @file UARTThread.h
 * @brief UART telemetry thread — sends sensor data to ESP32 at 10 Hz
 *
 * Two format options are provided:
 *
 * [ACTIVE] ASCII key:value CSV
 *   Format: CALLSIGN,ts:val,pa:val,tc:val,alt:val,sv:val,ax:val,...\r\n
 *   - Human-readable, easy to parse on the ground station
 *   - Ground station RX uses stale-value fallback per key — missing keys hold last known value
 *   - 'sv' field repurposed as flightState (no GPS on board)
 *   - 'it' field is 0.0 placeholder (ICM-45686 driver has no temperature output)
 *
 * [COMMENTED] Binary framed packet
 *   Format: [0xAA 0x55][len:2][payload:sizeof(TelemetryBundle)][checksum:1]
 *   - Compact, fixed-size, checksum-verified
 *   - Requires matching binary parser on the ESP32 side
 *   - Uses TelemetryBundle struct from FileLogStruct.h
 *   - Checksum = XOR of all payload bytes
 */

// ─── ACTIVE: ASCII key:value CSV ─────────────────────────────────────────────

void UARTThread(void *argument) {
    char buf[256];

    for (;;) {
        float accel_x = icm.accel_x / 2048.0f;
        float accel_y = icm.accel_y / 2048.0f;
        float accel_z = icm.accel_z / 2048.0f;
        float gyro_x  = icm.gyro_x  / 16.384f;
        float gyro_y  = icm.gyro_y  / 16.384f;
        float gyro_z  = icm.gyro_z  / 16.384f;

        int len = snprintf(buf, sizeof(buf),
            "%s,ts:%lu,pa:%ld,tc:%d,alt:%.1f,sv:%d,ax:%.3f,ay:%.3f,az:%.3f,gx:%.3f,gy:%.3f,gz:%.3f,it:%.1f\r\n",
            CALLSIGN,
            (unsigned long)HAL_GetTick(),
            (long)ms5607.getPressurePa(),
            (int)ms5607.getTemperatureC(),
            altitude,
            (int)flightState,
            accel_x, accel_y, accel_z,
            gyro_x,  gyro_y,  gyro_z,
            0.0f  // ICM45686_Handle has no temperature field; placeholder
        );


        if (len > 0) {
            HAL_UART_Transmit(&huart4, (uint8_t *)buf, (uint16_t)len, 100);
        }

        osDelay(300);
    }
}

// ─── ALTERNATIVE: Binary framed packet ───────────────────────────────────────
// Swap this in by commenting the ASCII version above and uncommenting below.
// ESP32 must parse: [0xAA][0x55][len_lo][len_hi][...payload...][xor_checksum]

//void UARTThread(void *argument) {
//    for (;;) {
//        TelemetryBundle pkt = {};
//        pkt.tick       = HAL_GetTick();
//        pkt.flightState           = (uint8_t)flightState;
//        pkt.filtered_altitude     = altitude;
//        pkt.filtered_acceleration = acceleration;
//        pkt.temperature = (int16_t)ms5607.getTemperatureC();
//        pkt.pressure    = (int32_t)ms5607.getPressurePa();
//        pkt.altitude    = altitude;
//        pkt.accel_x     = icm.accel_x / 2048.0f;
//        pkt.accel_y     = icm.accel_y / 2048.0f;
//        pkt.accel_z     = icm.accel_z / 2048.0f;
//        pkt.gyro_x      = icm.gyro_x  / 16.384f;
//        pkt.gyro_y      = icm.gyro_y  / 16.384f;
//        pkt.gyro_z      = icm.gyro_z  / 16.384f;

//
//        // XOR checksum over raw payload bytes
//        uint8_t checksum = 0;
//        uint8_t *p = (uint8_t *)&pkt;
//        for (size_t i = 0; i < sizeof(pkt); i++) checksum ^= p[i];
//
//        uint8_t header[4] = {
//            0xAA, 0x55,
//            (uint8_t)(sizeof(pkt) & 0xFF),
//            (uint8_t)((sizeof(pkt) >> 8) & 0xFF)
//        };
//
//        HAL_UART_Transmit(&huart4, header, sizeof(header), 50);
//        HAL_UART_Transmit(&huart4, (uint8_t *)&pkt, sizeof(pkt), 200);
//        HAL_UART_Transmit(&huart4, &checksum, 1, 10);
//
//        osDelay(300);
//    }
//}

#endif /* UARTTHREAD_H_ */
