# NMEA Parser Library for C++

## Overview

This C++ library provides functionality to parse NMEA sentences from GPS modules such as NEO-6M. It processes standard NMEA messages (GNRMC, GNGGA, GNGSA, GNVTG, GNGLL, GSV) and extracts relevant positioning information including latitude, longitude, altitude, speed, and more.

## Features

- Object-oriented design with intuitive API
- Support for multiple NMEA sentence types
- Automatic checksum validation
- Memory-safe implementation using C++ standard library
- Thoroughly documented code

## Supported NMEA Sentences

- **GNRMC**: Recommended Minimum Specific GNSS Data
- **GNGGA**: Global Positioning System Fixed Data
- **GNGLL**: Geographic Position - Latitude/Longitude
- **GNGSA**: GNSS DOP and Active Satellites
- **GNVTG**: Course Over Ground and Ground Speed
- **GSV**: GNSS Satellites in View (GPGSV, GLGSV, GBGSV, GAGSV, GQGSV)

## Installation

1. Add `nmea_parse.h` and `nmea_parse.cpp` to your project
2. Include the header file in your code:
   ```cpp
   #include "nmea_parse.h"
   ```

## Basic Usage

### Creating a Parser Object

```cpp
// Create a NMEA parser instance
NMEAParser parser;
```

### Parsing GPS Data

```cpp
// Assuming you have received NMEA data in buffer
uint8_t buffer[512]; // Buffer containing NMEA sentences from GPS module

// Parse the buffer
GPS gpsData = parser.parse(buffer);

// Now you can access the parsed GPS data
if (gpsData.hasFix()) {
    // We have a valid position fix
    double latitude = gpsData.getLatitude();
    char latSide = gpsData.getLatSide();
    double longitude = gpsData.getLongitude();
    char lonSide = gpsData.getLonSide();
    
    // Display position
    printf("Position: %.6f %c, %.6f %c\n", 
           latitude, latSide, 
           longitude, lonSide);
}
```

### Accessing All GPS Data

```cpp
// Check if we have a valid fix
if (gpsData.hasFix()) {
    printf("GPS Data:\n");
    printf("Position: %.6f %c, %.6f %c\n", 
           gpsData.getLatitude(), gpsData.getLatSide(),
           gpsData.getLongitude(), gpsData.getLonSide());
    printf("Altitude: %.1f meters\n", gpsData.getAltitude());
    printf("Speed: %.1f km/h\n", gpsData.getSpeed());
    printf("Course: %.1f degrees\n", gpsData.getCourse());
    printf("HDOP: %.1f\n", gpsData.getHDOP());
    printf("Satellites: %d\n", gpsData.getSatelliteCount());
    printf("UTC Time: %s\n", gpsData.getLastMeasure().c_str());
    printf("Date: %s\n", gpsData.getDate().c_str());
} else {
    printf("No valid GPS fix\n");
}
```

## Integration with STM32 UART

Here's an example of how to integrate the NMEA parser with an STM32 microcontroller using UART with DMA:

```cpp
// Declare buffer for UART DMA
uint8_t gpsBuffer[512];
GPS gpsData;
NMEAParser parser;

// Start DMA reception
HAL_UART_Receive_DMA(&huart2, gpsBuffer, sizeof(gpsBuffer));

// In your main loop or callback function
void processGPSData() {
    // Temporarily disable UART interrupts
    HAL_UART_DMAStop(&huart2);
    
    // Make sure buffer is null-terminated
    gpsBuffer[sizeof(gpsBuffer) - 1] = 0;
    
    // Parse the GPS data
    gpsData = parser.parse(gpsBuffer);
    
    // Process the parsed data
    if (gpsData.hasFix()) {
        // Use GPS position data
        // ...
    }
    
    // Clear buffer and restart DMA reception
    memset(gpsBuffer, 0, sizeof(gpsBuffer));
    HAL_UART_Receive_DMA(&huart2, gpsBuffer, sizeof(gpsBuffer));
}
```

## Examples

### Example 1: Basic Position Tracking

```cpp
#include "nmea_parse.h"
#include <iostream>

int main() {
    // Create test NMEA data
    const char* testData = "$GNRMC,123519.00,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n"
                           "$GNGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    
    NMEAParser parser;
    GPS gpsData = parser.parse(reinterpret_cast<const uint8_t*>(testData));
    
    if (gpsData.hasFix()) {
        std::cout << "Position: " << gpsData.getLatitude() << " " << gpsData.getLatSide() 
                  << ", " << gpsData.getLongitude() << " " << gpsData.getLonSide() << std::endl;
        std::cout << "Altitude: " << gpsData.getAltitude() << " meters" << std::endl;
        std::cout << "Speed: " << gpsData.getSpeed() << " km/h" << std::endl;
    } else {
        std::cout << "No valid GPS fix" << std::endl;
    }
    
    return 0;
}
```

### Example 2: Continuous GPS Monitoring

```cpp
#include "nmea_parse.h"
#include <iostream>
#include <chrono>
#include <thread>

// Mock function to simulate reading from UART
void readUARTData(uint8_t* buffer, size_t bufferSize) {
    // In a real application, this would read from UART
    const char* mockData = "$GNRMC,123519.00,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n";
    strcpy(reinterpret_cast<char*>(buffer), mockData);
}

int main() {
    NMEAParser parser;
    uint8_t buffer[512];
    
    // Simulate continuous monitoring
    for (int i = 0; i < 10; i++) {
        // Clear buffer
        memset(buffer, 0, sizeof(buffer));
        
        // Read data from UART (simulated)
        readUARTData(buffer, sizeof(buffer));
        
        // Parse GPS data
        GPS gpsData = parser.parse(buffer);
        
        // Process and display data
        if (gpsData.hasFix()) {
            std::cout << "Reading #" << i + 1 << std::endl;
            std::cout << "Position: " << gpsData.getLatitude() << " " << gpsData.getLatSide() 
                      << ", " << gpsData.getLongitude() << " " << gpsData.getLonSide() << std::endl;
            std::cout << "Time: " << gpsData.getLastMeasure() << std::endl;
            std::cout << "-----------------------" << std::endl;
        }
        
        // Wait before next reading
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

## API Reference

### GPS Class

The `GPS` class stores parsed GPS data and provides methods to access it.

#### Public Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `getLatitude()` | `double` | Gets the latitude in decimal degrees |
| `getLatSide()` | `char` | Gets the latitude hemisphere ('N' or 'S') |
| `getLongitude()` | `double` | Gets the longitude in decimal degrees |
| `getLonSide()` | `char` | Gets the longitude hemisphere ('E' or 'W') |
| `getAltitude()` | `float` | Gets the altitude in meters |
| `getHDOP()` | `float` | Gets the horizontal dilution of precision |
| `getSatelliteCount()` | `int` | Gets the number of satellites used |
| `hasFix()` | `bool` | Returns true if there's a valid GPS fix |
| `getLastMeasure()` | `std::string` | Gets the UTC time of the last fix |
| `getDate()` | `std::string` | Gets the date in DDMMYY format |
| `getSpeed()` | `float` | Gets the speed in km/h |
| `getCourse()` | `float` | Gets the course over ground in degrees |

### NMEAParser Class

The `NMEAParser` class provides methods to parse NMEA sentences.

#### Public Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `parse(const uint8_t* buffer)` | `GPS` | Parses NMEA data from the buffer and returns a GPS object |

## License

This library is provided as-is with no warranty. You are free to use and modify it for personal and commercial use.

## Version History

- 1.0.0 (2025-05-19): Initial release, converted from C to C++