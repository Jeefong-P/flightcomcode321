/*
 * ADXL375.h
 *
 *  Created on: Apr 8, 2024
 *      Author: Satvik Agrawal
 *  Converted to OOP: Object-Oriented C++ Version
 *  Updated: Added configurable CS pin support
 */

#ifndef ADXL375_H_
#define ADXL375_H_

#include "main.h" /* Needed for SPI */

/*
 * REGISTER ADDRESSES
 */
#define ADXL375_DEVID                0x00
#define ADXL375_THRESH_SHOCK         0x1D
#define ADXL375_OFFSET_X             0x1E
#define ADXL375_OFFSET_Y             0x1F
#define ADXL375_OFFSET_Z             0x20
#define ADXL375_SHOCK_DURATION       0x21
#define ADXL375_SHOCK_LATENT         0x22
#define ADXL375_SHOCK_WINDOW         0x23
#define ADXL375_ACT_THRESH           0x24
#define ADXL375_INACT_THRESH         0x25
#define ADXL375_TIME_INACT           0x26
#define ADXL375_ACT_INACT_CTL        0x27
#define ADXL375_SHOCK_AXES           0x2A
#define ADXL375_ACT_SHOCK_STAT       0x2B
#define ADXL375_BW_RATE              0x2C
#define ADXL375_POWER_CTL            0x2D
#define ADXL375_INT_ENABLE           0x2E
#define ADXL375_INT_MAP              0x2F
#define ADXL375_INT_SOURCE           0x30
#define ADXL375_DATA_FORMAT          0x31
#define ADXL375_DATAX0               0x32
#define ADXL375_DATAX1               0x33
#define ADXL375_DATAY0               0x34
#define ADXL375_DATAY1               0x35
#define ADXL375_DATAZ0               0x36
#define ADXL375_DATAZ1               0x37
#define ADXL375_FIFO_CTL             0x38
#define ADXL375_FIFO_STATUS          0x39

/*
 * INTERRUPT BITS
 */
#define ADXL375_INT_OVERRUN          0x01
#define ADXL375_INT_WATERMARK        0x02
#define ADXL375_INT_FREEFALL         0x04
#define ADXL375_INT_INACTIVITY       0x08
#define ADXL375_INT_ACTIVITY         0x10
#define ADXL375_INT_DOUBLE_TAP       0x20
#define ADXL375_INT_SINGLE_TAP       0x40
#define ADXL375_INT_DATA_READY       0x80

/*
 * ACTIVITY/INACTIVITY CONTROL BITS
 */
#define ADXL375_ACT_DC               0x80
#define ADXL375_ACT_X_EN             0x40
#define ADXL375_ACT_Y_EN             0x20
#define ADXL375_ACT_Z_EN             0x10
#define ADXL375_INACT_DC             0x08
#define ADXL375_INACT_X_EN           0x04
#define ADXL375_INACT_Y_EN           0x02
#define ADXL375_INACT_Z_EN           0x01

/*
 * REGISTER VALUES
 */
#define ADXL375_DEVID_WHOAMI         0xE5
#define ADXL375_POWER_CTL_MEASURE_MODE 0x08
#define ADXL375_BW_RATE_DATA_RATE    0x0F

/*
 * CONVERSION FACTOR
 */
#define ADXL375_SCALE_FACTOR         49.0f

class ADXL375 {
private:

    /*
     * MEMBER VARIABLES
     */
    SPI_HandleTypeDef *spiHandle;
    GPIO_TypeDef *csPort;
    uint16_t csPin;
    uint8_t rawAccData[6];
    float accData[3];

    /*
     * PRIVATE METHODS - Low-level functions
     */
    HAL_StatusTypeDef writeData(uint8_t address, uint8_t data, uint16_t len);
    HAL_StatusTypeDef readData(uint8_t address, uint8_t *data, uint16_t len);
    void toggleCSHigh();
    void toggleCSLow();
    void cleanRawValues();

public:
    /*
     * CONSTRUCTORS AND DESTRUCTOR
     */
    ADXL375();
    ADXL375(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *csPort, uint16_t csPin);
    ~ADXL375();

    /*
     * INITIALIZATION
     */
    int initialize(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *csPort, uint16_t csPin);
    bool isInitialized() const;

    /*
     * DATA ACQUISITION
     */
    HAL_StatusTypeDef readAcceleration();

    /*
     * GETTERS
     */
    float getAccX() const { return accData[0]; }
    float getAccY() const { return accData[1]; }
    float getAccZ() const { return accData[2]; }

    void getAcceleration(float &x, float &y, float &z) const;
    void getAccelerationArray(float acc[3]) const;

    // INTERUPT
    HAL_StatusTypeDef enableInterrupts(uint8_t interrupts);
    HAL_StatusTypeDef disableInterrupts(uint8_t interrupts);
    HAL_StatusTypeDef mapInterrupts(uint8_t interrupts, bool toINT2);
    uint8_t getInterruptSource();
    HAL_StatusTypeDef clearInterrupts();

    // Interrupt configuration helpers
    HAL_StatusTypeDef configureActivityInterrupt(uint8_t threshold, uint8_t axes, bool enable = true);
    HAL_StatusTypeDef configureInactivityInterrupt(uint8_t threshold, uint8_t time, uint8_t axes, bool enable = true);
    HAL_StatusTypeDef configureShockInterrupt(uint8_t threshold, uint8_t duration, uint8_t axes, bool enable = true);

    /*
     * UTILITY METHODS
     */
    bool devicePresent();
    HAL_StatusTypeDef setDataRate(uint8_t rate);
    HAL_StatusTypeDef setPowerMode(bool measureMode);
    HAL_StatusTypeDef setDataFormat(uint8_t format);

    /*
     * CONFIGURATION METHODS
     */
    HAL_StatusTypeDef setOffsets(int8_t offsetX, int8_t offsetY, int8_t offsetZ);
    HAL_StatusTypeDef setActivityThreshold(uint8_t threshold);
    HAL_StatusTypeDef setInactivityThreshold(uint8_t threshold);
};

#endif /* ADXL375_H_ */
