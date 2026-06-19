//
// Created by sztuka on 22.01.2023.
// Converted to C++ on 19.05.2025.
// Modified to remove std::string and convert GPS to struct
// Updated to format date as dd/mm/yyyy and time as hh:mm:ss.sss
//

#include "GNSS.h"
#include <cctype>

// NMEAParser constructor
NMEAParser::NMEAParser(UART_HandleTypeDef *huart) {
    this->huart = huart;
}

// NMEAParser destructor
NMEAParser::~NMEAParser() {
    // Empty destructor
}

// HELPER FUNCTIONS

// Tokenize a string into an array of strings using the specified delimiter
int NMEAParser::tokenize(const char* str, char delimiter, char tokens[][64], int maxTokens) {
    int tokenCount = 0;
    int start = 0;
    int len = strlen(str);

    for (int i = 0; i <= len && tokenCount < maxTokens; i++) {
        if (str[i] == delimiter || str[i] == '\0') {
            int tokenLen = i - start;
            if (tokenLen > 0 && tokenLen < 63) {
                strncpy(tokens[tokenCount], &str[start], tokenLen);
                tokens[tokenCount][tokenLen] = '\0';
            } else {
                tokens[tokenCount][0] = '\0';
            }
            tokenCount++;
            start = i + 1;
        }
    }

    return tokenCount;
}

// Calculate and verify NMEA checksum
bool NMEAParser::gpsChecksum(const char* nmeaData) {
    int len = strlen(nmeaData);

    // If string is too short, return false
    if (len < 5) return false;

    // Extract the checksum from the NMEA sentence (last 2 chars before \r\n)
    char recvCrc[3];
    strncpy(recvCrc, &nmeaData[len - 4], 2);
    recvCrc[2] = '\0';

    // Calculate the checksum
    int crc = 0;
    // Exclude the checksum part and its preceding '*'
    for (int i = 0; i < len - 5; i++) {
        crc ^= nmeaData[i];
    }

    // Convert received checksum from hex string to integer
    int receivedHash = (int)strtol(recvCrc, NULL, 16);

    return crc == receivedHash;
}

bool NMEAParser::isFloat(const char* str) {
    if (!str || strlen(str) == 0) return false;

    bool hasDecimal = false;
    bool hasDigit = false;
    int start = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    int len = strlen(str);

    for (int i = start; i < len; i++) {
        if (isdigit(str[i])) {
            hasDigit = true;
        } else if (str[i] == '.') {
            if (hasDecimal) return false;
            hasDecimal = true;
        } else {
            return false;
        }
    }

    return hasDigit;
}

bool NMEAParser::isInt(const char* str) {
    if (!str || strlen(str) == 0) return false;

    int start = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    int len = strlen(str);

    for (int i = start; i < len; i++) {
        if (!isdigit(str[i])) return false;
    }

    return (start < len); // at least one digit after sign
}

bool NMEAParser::strContains(const char* haystack, const char* needle) {
    return strstr(haystack, needle) != NULL;
}

void NMEAParser::strCopy(char* dest, const char* src, size_t maxLen) {
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0';
}

bool NMEAParser::convertLatLon(const char* field, char direction, bool isLatitude, double& result) {
    if (!field || strlen(field) < (isLatitude ? 4 : 5)) return false;

    int degLen = isLatitude ? 2 : 3;
    char degStr[4] = {0};
    strncpy(degStr, field, degLen);
    int degrees = atoi(degStr);
    float minutes = atof(field + degLen);
    result = degrees + minutes / 60.0;

    if (result == 0.0 || degrees > (isLatitude ? 90 : 180)) return false;

    if ((isLatitude && direction == 'S') || (!isLatitude && direction == 'W')) {
        result = -result;
    }

    return true;
}

// Parse time from HHMMSS.SS format into separate components
void NMEAParser::parseTime(const char* rawTime, uint8_t& hour, uint8_t& minute, uint8_t& second, uint16_t& millisecond) {
    if (!rawTime || strlen(rawTime) < 6) {
        hour = 0;
        minute = 0;
        second = 0;
        millisecond = 0;
        return;
    }

    // Extract hours, minutes
    char hours[3] = {rawTime[0], rawTime[1], '\0'};
    char minutes[3] = {rawTime[2], rawTime[3], '\0'};

    hour = (uint8_t)atoi(hours);
    minute = (uint8_t)atoi(minutes);

    // Extract seconds with decimal part
    const char* secondsStr = &rawTime[4];
    float fullSeconds = atof(secondsStr);

    // Split into integer seconds and fractional part
    second = (uint8_t)((int)fullSeconds);
    float fractionalPart = fullSeconds - second;

    // Convert fractional seconds to milliseconds (0.000-0.999 -> 0-999)
    millisecond = (uint16_t)(fractionalPart * 1000.0f + 0.5f); // +0.5f for rounding

    // Ensure millisecond doesn't exceed 999 due to rounding
    if (millisecond > 999) {
        millisecond = 999;
    }
}

// Parse date from DDMMYY format into separate components
void NMEAParser::parseDate(const char* rawDate, uint8_t& day, uint8_t& month, uint16_t& year) {
    if (!rawDate || strlen(rawDate) < 6) {
        day = 0;
        month = 0;
        year = 0;
        return;
    }

    // Extract day, month, year
    char dayStr[3] = {rawDate[0], rawDate[1], '\0'};
    char monthStr[3] = {rawDate[2], rawDate[3], '\0'};
    char yearStr[3] = {rawDate[4], rawDate[5], '\0'};

    day = (uint8_t)atoi(dayStr);
    month = (uint8_t)atoi(monthStr);

    // Convert 2-digit year to 4-digit year
    // Assume years 00-79 are 2000-2079, and 80-99 are 1980-1999
    int yearInt = atoi(yearStr);
    year = (uint16_t)((yearInt <= 79) ? 2000 + yearInt : 1900 + yearInt);
}

// PARSER FUNCTIONS

// Parse GNRMC sentence
bool NMEAParser::parseGNRMC(GPSData& gpsData, const char* sentence) {
    char tokens[MAX_TOKENS][64];
    int tokenCount = tokenize(sentence, ',', tokens, MAX_TOKENS);

    if (tokenCount < 12) return false;

    // Always parse time if available, regardless of fix status
    if (strlen(tokens[1]) >= 6) {
        parseTime(tokens[1], gpsData.hour, gpsData.minute, gpsData.second, gpsData.millisecond);
    }

    // Only parse position data if we have a valid fix (A = data valid)
    if (tokens[2][0] == 'A') {
        double latitude = 0.0, longitude = 0.0;
        char latSide = tokens[4][0];
        char lonSide = tokens[6][0];

        if (convertLatLon(tokens[3], latSide, true, latitude) &&
            convertLatLon(tokens[5], lonSide, false, longitude)) {

            gpsData.latitude = latitude;
            gpsData.latSide = latSide;
            gpsData.longitude = longitude;
            gpsData.lonSide = lonSide;
            gpsData.fix = true;

            // Speed in knots -> km/h
            if (isFloat(tokens[7])) {
                gpsData.speed = atof(tokens[7]) * 1.852f;
            }

            // Course over ground
            if (isFloat(tokens[8])) {
                gpsData.course = atof(tokens[8]);
            }

            // Format date from DDMMYY to separate components
            if (strlen(tokens[9]) >= 6) {
                parseDate(tokens[9], gpsData.day, gpsData.month, gpsData.year);
            }

            return true;
        }
    }

    return false;
}

bool NMEAParser::parseGNGGA(GPSData& gpsData, const char* sentence) {
    char tokens[MAX_TOKENS][64];
    int tokenCount = tokenize(sentence, ',', tokens, MAX_TOKENS);

    if (tokenCount < 10) return false;

    // Always parse time if available, regardless of fix status
    if (strlen(tokens[1]) >= 6) {
        parseTime(tokens[1], gpsData.hour, gpsData.minute, gpsData.second, gpsData.millisecond);
    }

    // Only parse position data if we have a valid fix (fix quality must be > 0)
    if (tokens[6][0] > '0') {
        double latitude = 0.0, longitude = 0.0;
        char latSide = tokens[3][0];
        char lonSide = tokens[5][0];

        if (convertLatLon(tokens[2], latSide, true, latitude) &&
            convertLatLon(tokens[4], lonSide, false, longitude)) {

            gpsData.latitude = latitude;
            gpsData.latSide = latSide;
            gpsData.longitude = longitude;
            gpsData.lonSide = lonSide;
            gpsData.fix = true;

            // Satellites
            if (isInt(tokens[7])) {
                gpsData.satelliteCount = atoi(tokens[7]);
            }

            // HDOP
            if (isFloat(tokens[8])) {
                gpsData.hdop = atof(tokens[8]);
            }

            // Altitude
            if (isFloat(tokens[9])) {
                gpsData.altitude = atof(tokens[9]);
            }

            return true;
        }
    }

    return false;
}

// Main parse method
void NMEAParser::parse(GPSData &gpsData, const uint8_t* buffer) {
    const char* bufferStr = reinterpret_cast<const char*>(buffer);

    // Process each sentence separated by '$'
    const char* start = bufferStr;
    const char* current = bufferStr;

    while (*current != '\0') {
        if (*current == '$') {
            // Found start of new sentence
            start = current + 1;
        } else if (*current == '\r' && *(current + 1) == '\n') {
            // Found end of sentence
            int sentenceLen = current - start;
            if (sentenceLen > 0 && sentenceLen < MAX_SENTENCE_LENGTH) {
                char sentence[MAX_SENTENCE_LENGTH];
                strncpy(sentence, start, sentenceLen);
                sentence[sentenceLen] = '\0';

                // Check which type of sentence it is and parse accordingly
                if (strContains(sentence, "GNRMC"))
                    parseGNRMC(gpsData, sentence);
                else if (strContains(sentence, "GNGGA"))
                    parseGNGGA(gpsData, sentence);
                // UNUSED parsers commented out
                // else if (strContains(sentence, "GNVTG"))
                //     parseGNVTG(gpsData, sentence);
                // else if (strContains(sentence, "GNGLL"))
                //     parseGNGLL(gpsData, sentence);
                // else if (strContains(sentence, "GNGSA"))
                //     parseGNGSA(gpsData, sentence);
                // else if (strContains(sentence, "GNGL") || strContains(sentence, "GLGSV") ||
                //          strContains(sentence, "GBGSV") || strContains(sentence, "GAGSV") ||
                //          strContains(sentence, "GQGSV") || strContains(sentence, "GPGSV"))
                //     parseGSV(gpsData, sentence);
            }
            current++; // Skip '\n'
        }
        current++;
    }
}

void NMEAParser::sendConfig(const uint8_t *config, size_t len) {
    HAL_UART_Transmit(huart, (uint8_t *) config, len, 1000);
}
