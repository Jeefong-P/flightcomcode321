# MS5607 C++ Class Implementation Guide

This document provides instructions on how to use the C++ class implementation of the MS5607-02 pressure sensor library.

## Notes
i2c and SPI up to 20 MHz

## Overview

The original C library has been converted to a C++ class to provide better encapsulation and object-oriented design while maintaining all the functionality from the original implementation.

## Files

- `MS5607.h` - Class declaration header file
- `MS5607.cpp` - Class implementation file

## Basic Usage

```cpp
// Include the header
#include "MS5607.h"

// Create an instance of the MS5607 class
MS5607 pressureSensor;

// In your initialization code:
SPI_HandleTypeDef hspi1; // Your SPI handle
GPIO_TypeDef* MS5607_CS_GPIO_Port = GPIOA; // Example GPIO port
uint16_t MS5607_CS_Pin = GPIO_PIN_4; // Example GPIO pin

void setup() {
    // Initialize SPI and GPIO (not shown here)
    
    // Initialize the MS5607 sensor
    if (pressureSensor.init(&hspi1, MS5607_CS_GPIO_Port, MS5607_CS_Pin) == MS5607_STATE_READY) {
        // Sensor initialized successfully
        
        // Optionally set oversampling ratio for better precision
        pressureSensor.setPressureOSR(OSR_4096);
        pressureSensor.setTemperatureOSR(OSR_4096);
    } else {
        // Sensor initialization failed
        // Handle error
    }
}

void loop() {
    // Update sensor readings
    pressureSensor.update();
    
    // Get temperature and pressure readings
    double temperature = pressureSensor.getTemperatureC();
    int32_t pressure = pressureSensor.getPressurePa();
    
    // Use the readings as needed
    
    // Wait before next reading
    HAL_Delay(100);
}
```

## Key Features

1. **Simple initialization**: Single `init()` method that sets up the sensor
2. **Object-oriented design**: Encapsulated functionality in a class
3. **Same functionality**: All features from the original C implementation preserved
4. **Improved readability**: Private methods and data structures for better organization

## Class Methods

### Public Methods

- `MS5607()` - Constructor, initializes class variables
- `MS5607StateTypeDef init(SPI_HandleTypeDef*, GPIO_TypeDef*, uint16_t)` - Initializes the sensor
- `void update()` - Updates sensor readings
- `double getTemperatureC()` - Returns temperature in Celsius
- `int32_t getPressurePa()` - Returns pressure in Pascal
- `void setTemperatureOSR(MS5607OSRFactors)` - Sets temperature oversampling ratio
- `void setPressureOSR(MS5607OSRFactors)` - Sets pressure oversampling ratio

### Private Methods

- `void enableCSB()` - Enables chip select
- `void disableCSB()` - Disables chip select
- `void readProm()` - Reads calibration data from PROM
- `void readUncompensatedValues()` - Reads raw values from ADC
- `void convertValues()` - Converts raw values to actual readings

## Oversampling Options

You can set the precision/speed tradeoff by using one of these oversampling ratios:

- `OSR_256` - Fastest conversion, lowest precision
- `OSR_512` - Medium-fast conversion
- `OSR_1024` - Medium conversion speed
- `OSR_2048` - Medium-high precision
- `OSR_4096` - Highest precision, slowest conversion

## Notes

- The class maintains all the original functionality including second-order temperature compensation
- The original SPI communication methods are preserved
- Error checking has been maintained from the original implementation
