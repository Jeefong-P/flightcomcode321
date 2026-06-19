# ADXL375 Main.c Implementation

## Basic Setup

```c
#include "main.h"
#include "ADXL375.h"

SPI_HandleTypeDef hspi1;
ADXL375 accelerometer;  // Create sensor object

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    // Initialize sensor
    int result = accelerometer.initialize(&hspi1);
    if (result != 0) {
        printf("Init failed: %d\n", result);
        Error_Handler();
    }
    
    // Configure sensor
    accelerometer.setDataRate(0x0D);     // 800 Hz
    accelerometer.setPowerMode(true);     // Enable measurement
    
    while(1) {
        // Read data
        if (accelerometer.readAcceleration() == HAL_OK) {
            // Get acceleration values
            float x = accelerometer.getAccX();
            float y = accelerometer.getAccY();
            float z = accelerometer.getAccZ();
            
            printf("X:%.2f Y:%.2f Z:%.2f mg\n", x, y, z);
        }
        HAL_Delay(100);
    }
}
```
## Interupt
```
// Enable data ready interrupt on INT1 pin
adxl.enableInterrupts(ADXL375_INT_DATA_READY);
adxl.mapInterrupts(ADXL375_INT_DATA_READY, false); // Map to INT1

// Configure activity interrupt (threshold=10, X+Y+Z axes enabled)
adxl.configureActivityInterrupt(10, ADXL375_ACT_X_EN | ADXL375_ACT_Y_EN | ADXL375_ACT_Z_EN, true);

// In your interrupt handler:
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == ADXL375_INT_PIN) {
        uint8_t intSource = adxl.getInterruptSource();
        
        if (intSource & ADXL375_INT_DATA_READY) {
            // Handle data ready
            adxl.readAcceleration();
        }
        
        if (intSource & ADXL375_INT_ACTIVITY) {
            // Handle activity detection
        }
    }
}
```


## Key Functions

| Function | Purpose |
|----------|---------|
| `accelerometer.initialize(&hspi1)` | Initialize with SPI handle |
| `accelerometer.readAcceleration()` | Read raw data and process |
| `accelerometer.getAccX/Y/Z()` | Get individual axis values |
| `accelerometer.devicePresent()` | Check if sensor connected |
| `accelerometer.setDataRate(rate)` | Configure sampling rate |

## Error Handling

```c
int initResult = accelerometer.initialize(&hspi1);
switch(initResult) {
    case 0:  // Success
        break;
    case -1: // Invalid handle
        printf("SPI handle error\n");
        break;
    case -2: // Device not found
        printf("Sensor not detected\n");
        break;
    default: // Communication error
        printf("Communication failed\n");
        break;
}
```

## Multiple Read Methods

```c
// Method 1: Individual getters
float x = accelerometer.getAccX();
float y = accelerometer.getAccY();
float z = accelerometer.getAccZ();

// Method 2: Reference parameters
float x, y, z;
accelerometer.getAcceleration(x, y, z);

// Method 3: Array
float acc[3];
accelerometer.getAccelerationArray(acc);
```

## Configuration Options

```c
accelerometer.setDataRate(0x0F);        // Max rate
accelerometer.setPowerMode(true);       // Enable measurement
accelerometer.setOffsets(0, 0, 0);      // Calibration
accelerometer.setDataFormat(0x0B);      // Full resolution
```

## Data Rate Settings

| Value | Rate | Description |
|-------|------|-------------|
| 0x0F | 3200 Hz | Maximum rate |
| 0x0E | 1600 Hz | High speed |
| 0x0D | 800 Hz | Recommended |
| 0x0C | 400 Hz | Medium |
| 0x0B | 200 Hz | Low power |
