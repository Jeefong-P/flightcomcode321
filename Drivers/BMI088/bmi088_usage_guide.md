# BMI088 IMU Driver Usage Guide

This guide provides comprehensive instructions for using the BMI088 6-axis IMU driver with STM32 HAL. The BMI088 combines a 3-axis accelerometer and 3-axis gyroscope, supporting both blocking and DMA-based operations via SPI.

## Table of Contents
- [Hardware Setup](#hardware-setup)
- [Basic Initialization](#basic-initialization)
- [Configuration Options](#configuration-options)
- [Reading Sensor Data](#reading-sensor-data)
- [DMA Operations](#dma-operations)
- [Interrupt Configuration](#interrupt-configuration)
- [Complete Example](#complete-example)
- [Troubleshooting](#troubleshooting)

## Hardware Setup

### Pin Connections
Connect the BMI088 to your STM32 microcontroller:

```
BMI088          STM32
VCC       -->   3.3V
GND       -->   GND
SCK       -->   SPI_SCK
MOSI      -->   SPI_MOSI
MISO      -->   SPI_MISO
CSA       -->   GPIO_PIN_A (Accelerometer CS)
CSG       -->   GPIO_PIN_B (Gyroscope CS)
INT1      -->   GPIO_PIN_C (Optional)
INT2      -->   GPIO_PIN_D (Optional)
INT3      -->   GPIO_PIN_E (Optional)
INT4      -->   GPIO_PIN_F (Optional)
```

### SPI Configuration
Configure SPI in STM32CubeMX with these settings:
- Mode: Master
- Frame Format: Motorola
- Data Size: 8 Bits
- First Bit: MSB First
- Clock Polarity: Low (CPOL = 0)
- Clock Phase: 2 Edge (CPHA = 1)
- NSS: Disabled (use GPIO for chip select)
- Baud Rate: Up to 10 MHz

### GPIO Configuration
Configure chip select pins as GPIO Output Push Pull, initially set HIGH.

## Basic Initialization

### 1. Include Headers and Create Instance

```cpp
#include "BMI088.h"

// Create BMI088 instance
BMI088 imu(&hspi1,                    // SPI handle
           GPIOA, GPIO_PIN_4,         // Accelerometer CS
           GPIOA, GPIO_PIN_5);        // Gyroscope CS
```

### 2. Initialize in main()

```cpp
int main(void) {
    // HAL initialization code...
    
    if (imu.initialize()) {
        printf("BMI088 initialized successfully\n");
    } else {
        printf("BMI088 initialization failed\n");
        // Handle error
    }
    
    while (1) {
        // Main loop
    }
}
```

## Configuration Options

### Accelerometer Configuration

```cpp
// Configure accelerometer
imu.setAccelerometerConfig(
    OVERSAMPLING_4,     // Oversampling for noise reduction
    ACC_ODR_400HZ,      // 400 Hz output data rate
    ACC_RANGE_6G        // ±6g measurement range
);
```

**Available Options:**

Oversampling:
- `OVERSAMPLING_4` - 4-fold oversampling (best noise performance)
- `OVERSAMPLING_2` - 2-fold oversampling
- `NORMAL` - No oversampling (lowest power)

Data Rates:
- `ACC_ODR_12_5HZ` to `ACC_ODR_1600HZ` (12.5 Hz to 1600 Hz)

Ranges:
- `ACC_RANGE_3G` (±3g) - Highest sensitivity
- `ACC_RANGE_6G` (±6g)
- `ACC_RANGE_12G` (±12g)
- `ACC_RANGE_24G` (±24g) - Lowest sensitivity

### Gyroscope Configuration

```cpp
// Configure gyroscope
imu.setGyroscopeConfig(
    GYR_ODR_400HZ_BW_47HZ,  // 400 Hz ODR, 47 Hz bandwidth
    GYR_RANGE_1000DPS       // ±1000°/s range
);
```

**Available Options:**

Data Rate and Bandwidth combinations:
- `GYR_ODR_2000HZ_BW_532HZ` - Highest rate, widest bandwidth
- `GYR_ODR_1000HZ_BW_116HZ`
- `GYR_ODR_400HZ_BW_47HZ`
- `GYR_ODR_200HZ_BW_23HZ`
- `GYR_ODR_100HZ_BW_12HZ` - Lowest rate, narrowest bandwidth

Ranges:
- `GYR_RANGE_125DPS` (±125°/s) - Highest sensitivity
- `GYR_RANGE_250DPS` (±250°/s)
- `GYR_RANGE_500DPS` (±500°/s)
- `GYR_RANGE_1000DPS` (±1000°/s)
- `GYR_RANGE_2000DPS` (±2000°/s) - Lowest sensitivity

## Reading Sensor Data

### Blocking Reads (Simple)

```cpp
void readSensorData() {
    // Read acceleration (blocking)
    Acceleration acc = imu.getAcceleration();
    printf("Accel: X=%.2f, Y=%.2f, Z=%.2f m/s²\n", 
           acc.x, acc.y, acc.z);
    
    // Read angular velocity (blocking)
    AngularVelocity gyro = imu.getAngularVelocity();
    printf("Gyro: X=%.2f, Y=%.2f, Z=%.2f */s\n", 
           gyro.x, gyro.y, gyro.z);
    
    // Read temperature
    float temp = imu.getTemperature();
    printf("Temperature: %.1f°C\n", temp);
    
    // Read sensor timestamp
    uint32_t sensorTime = imu.getSensorTime();
    printf("Sensor time: %lu\n", sensorTime);
}
```

### High-Frequency Polling

```cpp
void highFrequencyLoop() {
    while (1) {
        if (imu.isReady()) {
            Acceleration acc = imu.getAcceleration();
            AngularVelocity gyro = imu.getAngularVelocity();
            
            // Process data...
            processIMUData(acc, gyro);
        }
        
        // Small delay or other tasks
        HAL_Delay(1);
    }
}
```

## DMA Operations

DMA operations provide non-blocking, high-performance data acquisition ideal for real-time applications.

### 1. Setup DMA Callbacks

```cpp
// Global variables for data access
Acceleration latestAcceleration;
AngularVelocity latestAngularVelocity;
volatile bool newAccelData = false;
volatile bool newGyroData = false;

// Callback functions
void accelerometerDataReady() {
    latestAcceleration = imu.getAcceleration();
    newAccelData = true;
}

void gyroscopeDataReady() {
    latestAngularVelocity = imu.getAngularVelocity();
    newGyroData = true;
}
```

### 2. Register Callbacks and Start DMA

```cpp
void initializeDMA() {
    // Register callbacks
    imu.setAccelerometerCallback(accelerometerDataReady);
    imu.setGyroscopeCallback(gyroscopeDataReady);
    
    // Start initial DMA transfers
    imu.readAccelerometerDma();
    imu.readGyroscopeDma();
}
```

### 3. Handle DMA in Interrupt Handlers

Add these to your `stm32xxx_it.c` file:

```cpp
// External declaration
extern BMI088 imu;

void DMA1_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_spi1_rx);
}

void DMA1_Stream2_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_spi1_tx);
}

// DMA completion callbacks
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi1) {
        // Determine which sensor completed
        if (!imu.isAccelerometerReady()) {
            imu.handleAccelerometerDmaComplete();
        } else if (!imu.isGyroscopeReady()) {
            imu.handleGyroscopeDmaComplete();
        }
    }
}
```

### 4. Continuous DMA Loop

```cpp
void continuousDMALoop() {
    while (1) {
        // Check for new accelerometer data
        if (newAccelData) {
            newAccelData = false;
            
            // Process acceleration data
            processAcceleration(latestAcceleration);
            
            // Start next accelerometer reading
            if (imu.isAccelerometerReady()) {
                imu.readAccelerometerDma();
            }
        }
        
        // Check for new gyroscope data
        if (newGyroData) {
            newGyroData = false;
            
            // Process gyroscope data
            processAngularVelocity(latestAngularVelocity);
            
            // Start next gyroscope reading
            if (imu.isGyroscopeReady()) {
                imu.readGyroscopeDma();
            }
        }
        
        // Other tasks...
    }
}
```

## Interrupt Configuration

Configure hardware interrupts for event-driven data acquisition.

### Accelerometer Interrupts

```cpp
void configureAccelerometerInterrupts() {
    // Configure INT1 and INT2 pins
    imu.setAccelerometerInterrupt(
        OUTPUT_PIN,     // INT1 as output
        PUSH_PULL,      // Push-pull output
        ACTIVE_HIGH,    // Active high
        OUTPUT_PIN,     // INT2 as output  
        PUSH_PULL,      // Push-pull output
        ACTIVE_LOW      // Active low
    );
    
    // Map data ready interrupt to INT1
    imu.setAccelerometerInterruptMapData(
        INT1_DATA_READY,    // Data ready on INT1
        INT2_DISABLED       // INT2 disabled
    );
}
```

### Gyroscope Interrupts

```cpp
void configureGyroscopeInterrupts() {
    // Configure INT3 and INT4 pins
    imu.setGyroscopeInterrupt(
        PUSH_PULL,      // INT3 push-pull
        ACTIVE_HIGH,    // INT3 active high
        PUSH_PULL,      // INT4 push-pull
        ACTIVE_HIGH     // INT4 active high
    );
    
    // Map data ready interrupt to INT3
    imu.setGyroscopeInterruptMapData(
        INT3_DATA_READY,    // Data ready on INT3
        INT4_DISABLED       // INT4 disabled
    );
}
```

### Interrupt-Driven Data Reading

```cpp
// In your GPIO EXTI callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == ACCEL_INT_PIN) {
        // Accelerometer data ready
        if (imu.isAccelerometerReady()) {
            imu.readAccelerometerDma();
        }
    }
    
    if (GPIO_Pin == GYRO_INT_PIN) {
        // Gyroscope data ready
        if (imu.isGyroscopeReady()) {
            imu.readGyroscopeDma();
        }
    }
}
```

## Complete Example

Here's a complete example demonstrating various usage patterns:

```cpp
#include "main.h"
#include "BMI088.h"
#include <stdio.h>

// Global IMU instance
BMI088 imu(&hspi1, GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_5);

// Data storage
volatile Acceleration currentAccel;
volatile AngularVelocity currentGyro;
volatile bool dataReady = false;

// DMA callbacks
void onAccelReady() {
    currentAccel = imu.getAcceleration();
    dataReady = true;
}

void onGyroReady() {
    currentGyro = imu.getAngularVelocity();
}

int main(void) {
    // HAL initialization
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    
    printf("Starting BMI088 IMU Demo\n");
    
    // Initialize IMU
    if (!imu.initialize()) {
        printf("IMU initialization failed!\n");
        Error_Handler();
    }
    
    // Configure sensors
    imu.setAccelerometerConfig(OVERSAMPLING_4, ACC_ODR_800HZ, ACC_RANGE_6G);
    imu.setGyroscopeConfig(GYR_ODR_1000HZ_BW_116HZ, GYR_RANGE_1000DPS);
    
    // Setup DMA callbacks
    imu.setAccelerometerCallback(onAccelReady);
    imu.setGyroscopeCallback(onGyroReady);
    
    // Start DMA operations
    imu.readAccelerometerDma();
    imu.readGyroscopeDma();
    
    printf("IMU initialized successfully\n");
    
    uint32_t lastPrint = 0;
    
    while (1) {
        // Print data every 100ms
        if (HAL_GetTick() - lastPrint > 100) {
            if (dataReady) {
                printf("Accel: X=%.3f Y=%.3f Z=%.3f m/s²\n", 
                       currentAccel.x, currentAccel.y, currentAccel.z);
                printf("Gyro:  X=%.3f Y=%.3f Z=%.3f */s\n", 
                       currentGyro.x, currentGyro.y, currentGyro.z);
                printf("Temp:  %.1f°C\n", imu.getTemperature());
                printf("---\n");
                
                dataReady = false;
            }
            lastPrint = HAL_GetTick();
        }
        
        // Restart DMA if ready
        if (imu.isAccelerometerReady()) {
            imu.readAccelerometerDma();
        }
        if (imu.isGyroscopeReady()) {
            imu.readGyroscopeDma();
        }
        
        // Other application tasks
        HAL_Delay(1);
    }
}

// DMA completion handler
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi1) {
        if (!imu.isAccelerometerReady()) {
            imu.handleAccelerometerDmaComplete();
        } else if (!imu.isGyroscopeReady()) {
            imu.handleGyroscopeDmaComplete();
        }
    }
}
```

## Troubleshooting

### Common Issues and Solutions

**1. Initialization Fails**
- Check SPI connections and configuration
- Verify chip select pins are correctly configured
- Ensure proper power supply (3.3V)
- Check SPI clock polarity and phase settings

**2. Wrong Data Values**
- Verify sensor range configuration matches your application
- Check conversion factors in your calculations
- Ensure proper coordinate system orientation

**3. DMA Not Working**
- Verify DMA is properly configured in CubeMX
- Check interrupt handlers are correctly implemented
- Ensure DMA callbacks are registered before starting transfers

**4. Inconsistent Readings**
- Check for electrical noise on SPI lines
- Verify adequate power supply decoupling
- Consider lower SPI clock speeds
- Check for timing issues in interrupt handlers

**5. Performance Issues**
- Use DMA for high-frequency applications
- Avoid blocking operations in interrupt handlers
- Consider sensor data rates vs. processing capability

### Debug Tips

**Enable Debug Output:**
```cpp
// Add debug prints to track initialization
printf("Accel chip ID: 0x%02X (expected 0x1E)\n", chipId);
printf("Gyro chip ID: 0x%02X (expected 0x0F)\n", chipId);
```

**Monitor SPI Communication:**
Use an oscilloscope or logic analyzer to verify:
- Correct chip select timing
- Proper SPI clock and data signals
- Expected data patterns

**Check Sensor Status:**
```cpp
void printSensorStatus() {
    printf("Accelerometer ready: %s\n", 
           imu.isAccelerometerReady() ? "Yes" : "No");
    printf("Gyroscope ready: %s\n", 
           imu.isGyroscopeReady() ? "Yes" : "No");
    printf("Overall ready: %s\n", 
           imu.isReady() ? "Yes" : "No");
}
```

This guide covers the essential aspects of using the BMI088 driver. For advanced applications, refer to the BMI088 datasheet for detailed register descriptions and timing requirements.