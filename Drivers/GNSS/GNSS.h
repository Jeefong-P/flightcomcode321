//
// Created by sztuka on 22.01.2023.
// Converted to C++ on 19.05.2025.
// Modified to remove std::string and convert GPS to struct
// Updated to format date as dd/mm/yyyy and time as hh:mm:ss.sss
//

#ifndef STM32_SERIAL_DMA_NEO6M_PARSE_H
#define STM32_SERIAL_DMA_NEO6M_PARSE_H

#include "main.h"
#include <cstring>
#include <cstdlib>

#define MAX_TIME_LENGTH 16      // Legacy - kept for compatibility
#define MAX_DATE_LENGTH 12      // Legacy - kept for compatibility
#define MAX_SENTENCE_LENGTH 256
#define MAX_TOKENS 20

/**
 * @brief GPS data structure for storing parsed NMEA data
 */
typedef struct {
    double latitude;            // Latitude in degrees with decimal places
    char latSide;              // N or S
    double longitude;          // Longitude in degrees with decimal places
    char lonSide;              // E or W
    float altitude;            // Altitude in meters
    float hdop;                // Horizontal dilution of precision
    int satelliteCount;        // Number of satellites used in measurement
    bool fix;                  // true = fix, false = no fix

    // Time components
    uint8_t hour;              // Hour (0-23)
    uint8_t minute;            // Minute (0-59)
    uint8_t second;            // Second (0-59)
    uint16_t millisecond;      // Millisecond (0-999)

    // Date components
    uint8_t day;               // Day (1-31)
    uint8_t month;             // Month (1-12)
    uint16_t year;             // Year (full 4-digit year)

    float speed;               // Speed in km/h
    float course;              // Course over ground in degrees
} GPSData;

/**
 * @brief NMEA Parser class for processing GPS data
 */
class NMEAParser {
public:
    NMEAParser(UART_HandleTypeDef *huart);
    ~NMEAParser();

    /**
     * @brief Parses NMEA data from the GPS module
     * @param gpsData structure containing GPS data
     * @param buffer Pointer to buffer string with NMEA data
     */
    void parse(GPSData &gpsData, const uint8_t* buffer);

    void sendConfig(const uint8_t *config, size_t len);

private:
    // Helper methods for parsing different NMEA sentence types
    bool parseGNRMC(GPSData& gpsData, const char* sentence);
    bool parseGNGGA(GPSData& gpsData, const char* sentence);

    // Utility methods
    bool gpsChecksum(const char* nmeaData);
    int tokenize(const char* str, char delimiter, char tokens[][64], int maxTokens);
    bool isFloat(const char* str);
    bool isInt(const char* str);
    bool strContains(const char* haystack, const char* needle);
    void strCopy(char* dest, const char* src, size_t maxLen);
    bool convertLatLon(const char* field, char direction, bool isLatitude, double& result);
    void parseTime(const char* rawTime, uint8_t& hour, uint8_t& minute, uint8_t& second, uint16_t& millisecond);
    void parseDate(const char* rawDate, uint8_t& day, uint8_t& month, uint16_t& year);

    // variable
    UART_HandleTypeDef *huart;
};

#endif //STM32_SERIAL_DMA_NEO6M_PARSE_H
