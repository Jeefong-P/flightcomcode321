#ifndef GLOBALINCLUDE_H_
#define GLOBALINCLUDE_H_

#include "main.h"

/**
 * @brief System initialization status structure
 *
 * Tracks the ready state of all system components.
 * Each component sets its flag to 1 when successfully initialized.
 */
struct InitStatus {
    uint8_t ms5607Ready = 0;    ///< Barometric pressure sensor ready
    uint8_t icmReady = 0;       ///< IMU (accelerometer/gyroscope) ready
    uint8_t adxl375Ready = 0;   ///< High-G accelerometer ready
    uint8_t SDCardReady = 0;    ///< SD card storage ready
} initStatus;

/**
 * @brief Flight state enumeration
 *
 * Represents the current phase of flight for state machine control.
 * States progress sequentially during normal flight operations.
 */
enum FlightState : uint8_t {
    IDLE      = 0,  ///< On ground, ready for launch
    BOOST     = 1,  ///< Motor burn phase
    COAST     = 2,  ///< Post-burnout: phase 1 acc-trusting, phase 2 baro-trusting
    APOGEE    = 3,  ///< At maximum altitude
    DROGUE    = 4,  ///< Drogue chute deployed
    MAIN      = 5,  ///< Main chute deployed
    TOUCHDOWN = 6   ///< Landed
};

FlightState flightState = IDLE;

/**
 * @brief Check if all system components are ready
 * @return true if all components initialized successfully
 */
bool initStatusAllReady() {
    return (initStatus.ms5607Ready &&
            initStatus.icmReady &&
            initStatus.SDCardReady);
}

#endif /* GLOBALINCLUDE_H_ */
