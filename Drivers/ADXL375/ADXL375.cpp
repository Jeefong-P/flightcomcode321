/*
 * ADXL375.cpp
 *
 *  Created on: Apr 8, 2024
 *      Author: HP
 *  Converted to OOP: Object-Oriented C++ Version
 *  Updated: Added configurable CS pin support
 */

#include "ADXL375.h"

/*
 * CONSTRUCTORS AND DESTRUCTOR
 */
ADXL375::ADXL375() : spiHandle(nullptr), csPort(nullptr), csPin(0) {
    for (int i = 0; i < 6; i++) {
        rawAccData[i] = 0;
    }

    for (int i = 0; i < 3; i++) {
        accData[i] = 0.0f;
    }
}

ADXL375::ADXL375(SPI_HandleTypeDef *handle, GPIO_TypeDef *port, uint16_t pin) : ADXL375() {
    initialize(handle, port, pin);
}

ADXL375::~ADXL375() {
    if (spiHandle != nullptr && csPort != nullptr) {
        toggleCSHigh();
    }
}

/*
 * INITIALIZATION
 */
int ADXL375::initialize(SPI_HandleTypeDef *handle, GPIO_TypeDef *port, uint16_t pin) {
    if (handle == nullptr || port == nullptr) {
        return -1;
    }

    // Set CS pin high initially (inactive)
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

    spiHandle = handle;
    csPort = port;
    csPin = pin;

    HAL_StatusTypeDef status;
    int errorNum = 0;

    /* Confirm device address as 0xE5 */
    uint8_t confirmID;
    int attempts = 0;
    const int maxAttempts = 100;

    while (attempts < maxAttempts) {
        status = readData(ADXL375_DEVID, &confirmID, 1);
        if (status == HAL_OK && confirmID == ADXL375_DEVID_WHOAMI) {
            break;
        }
        attempts++;
        HAL_Delay(10);
    }

    if (attempts >= maxAttempts) {
        return -2; // Device not found
    }

    /* Set the required data rate */
    status = writeData(ADXL375_BW_RATE, ADXL375_BW_RATE_DATA_RATE, 1);
    if (status != HAL_OK) {
        errorNum++;
    }

    /* Enable measurement mode */
    status = writeData(ADXL375_POWER_CTL, ADXL375_POWER_CTL_MEASURE_MODE, 1);
    if (status != HAL_OK) {
        errorNum++;
    }

    /* Set data format */
    status = writeData(ADXL375_DATA_FORMAT, ADXL375_POWER_CTL_MEASURE_MODE, 1);
    if (status != HAL_OK) {
        errorNum++;
    }

    return errorNum;
}

bool ADXL375::isInitialized() const {
    return (spiHandle != nullptr && csPort != nullptr);
}

/*
 * DATA ACQUISITION
 */
HAL_StatusTypeDef ADXL375::readAcceleration() {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = readData(ADXL375_DATAX0, rawAccData, 6);

    if (status == HAL_OK) {
        cleanRawValues();
    }

    return status;
}

/*
 * GETTERS
 */
void ADXL375::getAcceleration(float &x, float &y, float &z) const {
    x = accData[0];
    y = accData[1];
    z = accData[2];
}

void ADXL375::getAccelerationArray(float acc[3]) const {
    acc[0] = accData[0];
    acc[1] = accData[1];
    acc[2] = accData[2];
}

/*
 * UTILITY METHODS
 */
bool ADXL375::devicePresent() {
    if (!isInitialized()) {
        return false;
    }

    uint8_t deviceId;
    HAL_StatusTypeDef status = readData(ADXL375_DEVID, &deviceId, 1);

    return (status == HAL_OK && deviceId == ADXL375_DEVID_WHOAMI);
}

HAL_StatusTypeDef ADXL375::setDataRate(uint8_t rate) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(ADXL375_BW_RATE, rate, 1);
}

HAL_StatusTypeDef ADXL375::setPowerMode(bool measureMode) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    uint8_t value = measureMode ? ADXL375_POWER_CTL_MEASURE_MODE : 0x00;
    return writeData(ADXL375_POWER_CTL, value, 1);
}

HAL_StatusTypeDef ADXL375::setDataFormat(uint8_t format) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(ADXL375_DATA_FORMAT, format, 1);
}

/*
 * CONFIGURATION METHODS
 */
HAL_StatusTypeDef ADXL375::setOffsets(int8_t offsetX, int8_t offsetY, int8_t offsetZ) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = writeData(ADXL375_OFFSET_X, static_cast<uint8_t>(offsetX), 1);
    if (status != HAL_OK) return status;

    status = writeData(ADXL375_OFFSET_Y, static_cast<uint8_t>(offsetY), 1);
    if (status != HAL_OK) return status;

    status = writeData(ADXL375_OFFSET_Z, static_cast<uint8_t>(offsetZ), 1);
    return status;
}

HAL_StatusTypeDef ADXL375::setActivityThreshold(uint8_t threshold) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(ADXL375_ACT_THRESH, threshold, 1);
}

HAL_StatusTypeDef ADXL375::setInactivityThreshold(uint8_t threshold) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(ADXL375_INACT_THRESH, threshold, 1);
}

/*
 * PRIVATE METHODS
 */
void ADXL375::cleanRawValues() {
    int16_t val;

    /* CONVERSION FOR ACC_X */
    val = static_cast<int16_t>((rawAccData[1] << 8) | rawAccData[0]);
    accData[0] = static_cast<float>(val) * ADXL375_SCALE_FACTOR;

    /* CONVERSION FOR ACC_Y */
    val = static_cast<int16_t>((rawAccData[3] << 8) | rawAccData[2]);
    accData[1] = static_cast<float>(val) * ADXL375_SCALE_FACTOR;

    /* CONVERSION FOR ACC_Z */
    val = static_cast<int16_t>((rawAccData[5] << 8) | rawAccData[4]);
    accData[2] = static_cast<float>(val) * ADXL375_SCALE_FACTOR;
}

HAL_StatusTypeDef ADXL375::writeData(uint8_t address, uint8_t data, uint16_t len) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    toggleCSLow();

    HAL_StatusTypeDef status = HAL_SPI_Transmit(spiHandle, &address, 1, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        status = HAL_SPI_Transmit(spiHandle, &data, len, HAL_MAX_DELAY);
    }

    toggleCSHigh();
    return status;
}

HAL_StatusTypeDef ADXL375::readData(uint8_t address, uint8_t *data, uint16_t len) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    toggleCSLow();

    uint8_t txBuffer = (address | 0x80);

    if (len > 1) {
        txBuffer = (txBuffer | 0x40); // Multi-byte read
    }

    HAL_StatusTypeDef status = HAL_SPI_Transmit(spiHandle, &txBuffer, 1, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        status = HAL_SPI_Receive(spiHandle, data, len, HAL_MAX_DELAY);
    }

    toggleCSHigh();
    return status;
}

/*
 * INTERRUPT METHODS
 */
HAL_StatusTypeDef ADXL375::enableInterrupts(uint8_t interrupts) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(ADXL375_INT_ENABLE, interrupts, 1);
}

HAL_StatusTypeDef ADXL375::disableInterrupts(uint8_t interrupts) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    uint8_t currentValue;
    HAL_StatusTypeDef status = readData(ADXL375_INT_ENABLE, &currentValue, 1);
    if (status != HAL_OK) return status;

    currentValue &= ~interrupts;
    return writeData(ADXL375_INT_ENABLE, currentValue, 1);
}

HAL_StatusTypeDef ADXL375::mapInterrupts(uint8_t interrupts, bool toINT2) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    uint8_t mapValue = toINT2 ? interrupts : 0x00;
    return writeData(ADXL375_INT_MAP, mapValue, 1);
}

uint8_t ADXL375::getInterruptSource() {
    if (!isInitialized()) {
        return 0;
    }

    uint8_t intSource;
    HAL_StatusTypeDef status = readData(ADXL375_INT_SOURCE, &intSource, 1);

    return (status == HAL_OK) ? intSource : 0;
}
HAL_StatusTypeDef ADXL375::clearInterrupts() {
    return (getInterruptSource() != 0) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef ADXL375::configureActivityInterrupt(uint8_t threshold, uint8_t axes, bool enable) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    // Set activity threshold
    HAL_StatusTypeDef status = writeData(ADXL375_ACT_THRESH, threshold, 1);
    if (status != HAL_OK) return status;

    // Configure activity/inactivity control
    uint8_t controlValue = axes & 0x70; // Activity axes (bits 6,5,4)
    status = writeData(ADXL375_ACT_INACT_CTL, controlValue, 1);
    if (status != HAL_OK) return status;

    // Enable/disable activity interrupt
    if (enable) {
        return enableInterrupts(ADXL375_INT_ACTIVITY);
    } else {
        return disableInterrupts(ADXL375_INT_ACTIVITY);
    }
}

HAL_StatusTypeDef ADXL375::configureInactivityInterrupt(uint8_t threshold, uint8_t time, uint8_t axes, bool enable) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    // Set inactivity threshold
    HAL_StatusTypeDef status = writeData(ADXL375_INACT_THRESH, threshold, 1);
    if (status != HAL_OK) return status;

    // Set inactivity time
    status = writeData(ADXL375_TIME_INACT, time, 1);
    if (status != HAL_OK) return status;

    // Configure activity/inactivity control
    uint8_t controlValue = axes & 0x07; // Inactivity axes (bits 2,1,0)
    status = writeData(ADXL375_ACT_INACT_CTL, controlValue, 1);
    if (status != HAL_OK) return status;

    // Enable/disable inactivity interrupt
    if (enable) {
        return enableInterrupts(ADXL375_INT_INACTIVITY);
    } else {
        return disableInterrupts(ADXL375_INT_INACTIVITY);
    }
}

HAL_StatusTypeDef ADXL375::configureShockInterrupt(uint8_t threshold, uint8_t duration, uint8_t axes, bool enable) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    // Set shock threshold
    HAL_StatusTypeDef status = writeData(ADXL375_THRESH_SHOCK, threshold, 1);
    if (status != HAL_OK) return status;

    // Set shock duration
    status = writeData(ADXL375_SHOCK_DURATION, duration, 1);
    if (status != HAL_OK) return status;

    // Configure shock axes
    status = writeData(ADXL375_SHOCK_AXES, axes, 1);
    if (status != HAL_OK) return status;

    // Enable/disable shock interrupt
    if (enable) {
        return enableInterrupts(ADXL375_INT_SINGLE_TAP);
    } else {
        return disableInterrupts(ADXL375_INT_SINGLE_TAP);
    }
}


void ADXL375::toggleCSHigh() {
    if (csPort != nullptr) {
        HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
    }
}

void ADXL375::toggleCSLow() {
    if (csPort != nullptr) {
        HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    }
}

