/**
 * @file FlightLogicThread.h
 * @brief Main flight state machine and control logic
 * @author Korn
 * @date June 2025
 * @lastest update by Golf, June 17, 2026
 *
 * This file contains the core flight logic that manages the rocket's flight states
 * from launch detection through touchdown. It processes sensor data and makes
 * critical flight decisions including parachute deployment timing.
 */

#ifndef FLIGHT_LOGIC_THREAD_H
#define FLIGHT_LOGIC_THREAD_H

#include "ICM.h"
#include "cmsis_os2.h"
#include "MS5607.h"
#include "FileLogStruct.h"
#include "GlobalInclude.h"
#include "SDCardThread.h"
#include "SimpleKalmanFilter.h"

#define DEG_TO_RAD M_PI / 180.0f

// External sensor instances
extern MS5607 ms5607;
extern TIM_HandleTypeDef htim2;

// Flight detection parameters
#define BOOST_ACCELERATION_THRESHOLD  100.0f        // m/s² - 3 consecutive samples to detect launch
#define MIN_BOOST_TIME                3000          // ms  - motor must burn at least this long
#define MAX_BOOST_TIME                15000         // ms  - failsafe: force COAST if IMU stuck in boost
#define COAST_ACCELERATION_THRESHOLD  50.0f         // m/s² - burnout detection
#define COAST_TIME_LOCK               (20 * 1000)   // ms  - minimum coast time before checking apogee (covers mach transition)
#define COAST_MAX_TIME                (60 * 1000)   // ms  - failsafe: force APOGEE if baro never shows drop
#define MAX_ALTITUDE_DIFF             15.0f         // m   - altitude drop below max to start apogee confirm
#define APOGEE_CONFIRM_MS             4000          // ms  - descent must sustain this long before confirming apogee
#define DROGUE_TIME_LOCK              (200 * 1000)  // ms  - minimum time after apogee before main deploy
#define MAIN_FAILSAFE_MS              (300 * 1000)  // ms  - hard deadline: deploy main regardless of altitude
#define MAIN_DEPLOYMENT_ALTITUDE      400.0f        // m   - altitude gate for main chute
#define MIN_APOGEE_ALTITUDE           1000.0f       // m   - minimum altitude to accept as valid apogee
#define TOUCHDOWN_ALTITUDE_GATE       50.0f         // m   - only check touchdown below this altitude
#define MAX_ALT_JUMP_PER_SAMPLE       100.0f        // m   - reject baro spike larger than this for maxAltitude

// Global flight data (filtered)
float altitude = 0.0f;
float acceleration = 0.0f;

uint16_t stillAltitudeCount = 0;
uint8_t  consecutiveHighG  = 0;
uint32_t descentStartTick  = 0;

/**
 * @brief Wait for all subsystems to initialize (max 5 seconds)
 * @note Blocks until all systems ready or timeout occurs
 */
void waitInit() {
    const uint32_t INIT_TIMEOUT_MS = 5000;
    uint32_t startTick = HAL_GetTick();

    while ((HAL_GetTick() - startTick) <= INIT_TIMEOUT_MS) {
        if (initStatusAllReady()) {
            break;
        }
        osDelay(1);
    }
}

/**
 * @brief Calculate microsecond time difference with overflow handling
 * @param lastMicroSecond Reference to last timestamp (updated)
 * @return Time difference in seconds (microseconds resolution)
 */
double getMicroSecondTimeDiff(uint32_t &lastMicroSecond) {
    uint32_t currentTime = __HAL_TIM_GET_COUNTER(&htim2);

    if (currentTime >= lastMicroSecond) {
        double diff = currentTime - lastMicroSecond;
        lastMicroSecond = currentTime;
        return diff / 1000000.0;
    } else {
        // Handle timer overflow
        double diff = currentTime + (0xFFFF - lastMicroSecond);
        lastMicroSecond = currentTime;
        return diff / 1000000.0;
    }
}


/**
 * @brief Main flight logic thread - handles flight state machine
 * @param argument Thread parameter (unused)
 *
 * State machine: IDLE → BOOST → COAST → APOGEE → DROGUE → MAIN → TOUCHDOWN
 *
 * COAST has two internal phases separated by COAST_TIME_LOCK:
 *   Phase 1 (0–20s): acc-trusting — covers mach transition, baro ignored
 *   Phase 2 (20s+):  baro-trusting — apogee detection active
 */
void FlightLogicThread(void *argument) {
    waitInit();

    HAL_TIM_Base_Start(&htim2);

    uint32_t boostStartTick  = 0;
    uint32_t coastStartTick  = 0;
    uint32_t apogeeStartTick = 0;
    float maxAltitude  = 0.0f;
    float lastAltitude = 0.0f;

    if (!initStatus.ms5607Ready || !initStatus.icmReady) {
        while (1) { osDelay(1000); }
    }

    for (;;) {
        switch (flightState) {

            case IDLE:
                if (acceleration > BOOST_ACCELERATION_THRESHOLD) {
                    ++consecutiveHighG;
                    if (consecutiveHighG >= 3) {
                        flightState = BOOST;
                        boostStartTick = HAL_GetTick();
                    }
                } else {
                    consecutiveHighG = 0;
                }
                break;

            case BOOST:
                // Normal burnout: acceleration drops after min burn time
                if (acceleration < COAST_ACCELERATION_THRESHOLD &&
                    HAL_GetTick() - boostStartTick > MIN_BOOST_TIME) {
                    coastStartTick = HAL_GetTick();
                    maxAltitude = altitude;
                    flightState = COAST;
                }
                // Failsafe: IMU stuck high — force coast after max burn time
                if (HAL_GetTick() - boostStartTick >= MAX_BOOST_TIME) {
                    coastStartTick = HAL_GetTick();
                    maxAltitude = altitude;
                    flightState = COAST;
                }
                break;

            case COAST:
                // Phase 1 (acc-trusting): COAST_TIME_LOCK covers mach transition — baro ignored here
                // Phase 2 (baro-trusting): after lock expires, watch for sustained altitude drop

                // Spike-filtered maxAltitude: ignore single-sample jumps > 100m
                if (altitude > maxAltitude && (altitude - maxAltitude) < MAX_ALT_JUMP_PER_SAMPLE) {
                    maxAltitude = altitude;
                }

                if ((HAL_GetTick() - coastStartTick) > COAST_TIME_LOCK &&
                    altitude >= MIN_APOGEE_ALTITUDE) {

                    if ((maxAltitude - altitude) > MAX_ALTITUDE_DIFF) {
                        // Require sustained descent before confirming apogee
                        if (descentStartTick == 0) descentStartTick = HAL_GetTick();
                        if (HAL_GetTick() - descentStartTick >= APOGEE_CONFIRM_MS) {
                            descentStartTick = 0;
                            flightState = APOGEE;
                        }
                    } else {
                        // Altitude recovered — not real apogee, reset window
                        descentStartTick = 0;
                    }
                }

                // Failsafe: baro broken or too noisy — force apogee after max coast time
                if (HAL_GetTick() - coastStartTick >= COAST_MAX_TIME) {
                    descentStartTick = 0;
                    flightState = APOGEE;
                }
                break;

            case APOGEE:
                // Pyro handled by separate board — record time and advance
                apogeeStartTick = HAL_GetTick();
                flightState = DROGUE;
                break;

            case DROGUE:
                // Normal: altitude-gated after minimum drogue time
                if (HAL_GetTick() - apogeeStartTick >= DROGUE_TIME_LOCK &&
                    altitude < MAIN_DEPLOYMENT_ALTITUDE) {
                    lastAltitude = altitude;
                    flightState = MAIN;
                }
                // Failsafe: baro stuck above 1000m — deploy main on hard deadline
                break;

            case MAIN:
                // Only count stable readings when close to ground — prevents false
                // touchdown during slow main chute descent at altitude
                if (altitude < TOUCHDOWN_ALTITUDE_GATE) {
                    if (fabsf(altitude - lastAltitude) < 1.0f) {
                        ++stillAltitudeCount;
                        if (stillAltitudeCount >= 1000) {
                            flightState = TOUCHDOWN;
                        }
                    } else {
                        stillAltitudeCount = 0;
                    }
                } else {
                    stillAltitudeCount = 0;
                }
                lastAltitude = altitude;
                break;

            case TOUCHDOWN:
                osDelay(1000);
                break;
        }

        osDelay(1);
    }
}

/**
 * @brief Flight data logging thread
 * @note Logs flight state and filtered sensor data to SD card
 */
void FlightLogicLogThread(void *argument) {
    for (;;) {
        FlightLogicData data = {
            .tick = HAL_GetTick(),
            .flightState = flightState,
            .filtered_altitude = altitude,
            .filtered_acceleration = acceleration
        };

        flightLogicLogger.logStruct(&data, sizeof(FlightLogicData));

        if (flightState == IDLE) {
            osDelay(10);        // 100 Hz during idle
        } else if (flightState == TOUCHDOWN) {
            osDelay(1000);      // 1 Hz after touchdown
        } else {
            osDelay(1);         // 500 Hz during active flight
        }
    }
}

#endif // FLIGHT_LOGIC_THREAD_H
