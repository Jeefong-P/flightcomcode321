#ifndef UARTTHREAD_H_
#define UARTTHREAD_H_

#include "cmsis_os2.h"
#include "stm32u5xx_hal.h"
#include "GlobalInclude.h"
#include <string.h>

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
 * [COMMENTED] ASCII key:value CSV
 *   Format: CALLSIGN,ts:val,pa:val,tc:val,alt:val,sv:val,ax:val,...\r\n
 *   - Human-readable, easy to parse on the ground station
 *   - Ground station RX uses stale-value fallback per key — missing keys hold last known value
 *   - 'sv' field repurposed as flightState (no GPS on board)
 *
 * [ACTIVE] Binary framed packet
 *   Format: [0xAA 0x55][len:2][payload:sizeof(UARTPacket)][checksum:1]
 *   - Compact, fixed-size, checksum-verified
 *   - Single HAL_UART_Transmit call — no fragmentation risk under RTOS context switch
 *   - Checksum = XOR of all payload bytes
 */

// ─── ALTERNATIVE: ASCII key:value CSV ────────────────────────────────────────
// Swap this in by commenting the binary version below and uncommenting here.

//void UARTThread(void *argument) {
//    char buf[256];
//
//    for (;;) {
//        float accel_x = icm.accel_x / 2048.0f;
//        float accel_y = icm.accel_y / 2048.0f;
//        float accel_z = icm.accel_z / 2048.0f;
//        float gyro_x  = icm.gyro_x  / 16.384f;
//        float gyro_y  = icm.gyro_y  / 16.384f;
//        float gyro_z  = icm.gyro_z  / 16.384f;
//
//        int len = snprintf(buf, sizeof(buf),
//            "%s,ts:%lu,pa:%ld,tc:%d,alt:%.1f,sv:%d,ax:%.3f,ay:%.3f,az:%.3f,gx:%.3f,gy:%.3f,gz:%.3f\r\n",
//            CALLSIGN,
//            (unsigned long)HAL_GetTick(),
//            (long)ms5607.getPressurePa(),
//            (int)ms5607.getTemperatureC(),
//            altitude,
//            (int)flightState,
//            accel_x, accel_y, accel_z,
//            gyro_x,  gyro_y,  gyro_z
//        );
//
//        if (len > 0) {
//            HAL_UART_Transmit(&huart4, (uint8_t *)buf, (uint16_t)len, 100);
//        }
//
//        osDelay(300);
//    }
//}

// ─── ACTIVE: Binary framed packet ────────────────────────────────────────────
// Frame: [0xAA][0x55][len_lo][len_hi][...payload...][xor_checksum]
// ESP32 parser: wait for 0xAA 0x55, read 2-byte length, read payload, verify XOR.
//
// Payload struct layout:
//   char     callsign[7]  — CALLSIGN + null terminator
//   uint32_t tick
//   int32_t  pressure     — Pa
//   int16_t  temperature  — °C
//   float    altitude     — m (Kalman-filtered)
//   uint8_t  flightState
//   float    accel_x, accel_y, accel_z  — g
//   float    gyro_x,  gyro_y,  gyro_z   — dps

struct __attribute__((packed)) UARTPacket {
    char     callsign[7];
    uint32_t tick;
    int32_t  pressure;
    int16_t  temperature;
    float    altitude;
    uint8_t  flightState;
    float    accel_x, accel_y, accel_z;
    float    gyro_x,  gyro_y,  gyro_z;
};

void UARTThread(void *argument) {
    for (;;) {
        UARTPacket pkt = {};
        strncpy(pkt.callsign, CALLSIGN, sizeof(pkt.callsign));
        pkt.tick        = HAL_GetTick();
        pkt.pressure    = (int32_t)ms5607.getPressurePa();
        pkt.temperature = (int16_t)ms5607.getTemperatureC();
        pkt.altitude    = altitude;
        pkt.flightState = (uint8_t)flightState;
        pkt.accel_x     = icm.accel_x / 2048.0f;
        pkt.accel_y     = icm.accel_y / 2048.0f;
        pkt.accel_z     = icm.accel_z / 2048.0f;
        pkt.gyro_x      = icm.gyro_x  / 16.384f;
        pkt.gyro_y      = icm.gyro_y  / 16.384f;
        pkt.gyro_z      = icm.gyro_z  / 16.384f;

        // XOR checksum over payload bytes
        uint8_t checksum = 0;
        uint8_t *p = (uint8_t *)&pkt;
        for (size_t i = 0; i < sizeof(pkt); i++) checksum ^= p[i];

        // Single buffer — one HAL_UART_Transmit call prevents RTOS context switch gaps
        uint8_t frame[4 + sizeof(UARTPacket) + 1];
        frame[0] = 0xAA;
        frame[1] = 0x55;
        frame[2] = (uint8_t)(sizeof(UARTPacket) & 0xFF);
        frame[3] = (uint8_t)((sizeof(UARTPacket) >> 8) & 0xFF);
        memcpy(&frame[4], p, sizeof(UARTPacket));
        frame[4 + sizeof(UARTPacket)] = checksum;

        HAL_UART_Transmit(&huart4, frame, sizeof(frame), 200);

        osDelay(300);
    }
}

#endif /* UARTTHREAD_H_ */
