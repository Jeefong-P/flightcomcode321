#ifndef FILELOGSTRUCT_H_
#define FILELOGSTRUCT_H_

/**
 * @brief Barometric pressure sensor data structure
 *
 * Contains pressure and temperature measurements from MS5607 sensor.
 * Used for altitude calculation and atmospheric monitoring.
 */
struct MS5607Data {
    uint32_t tick;          ///< System timestamp in milliseconds
    int16_t temperature;    ///< Temperature in degrees Celsius * 100
    int32_t pressure;       ///< Atmospheric pressure in Pascals
};

struct ICMAccelerationData {
    uint32_t tick;
    float accX, accY, accZ;
};

struct ICMGyroData {
    uint32_t tick;
    float gyX, gyY, gyZ;
};

/**
 * @brief ADXL375 high-G accelerometer data structure
 *
 * High-range acceleration data for detecting high-G events.
 * Typically used during motor burn and high acceleration phases.
 */
struct ADXL375Data {
    uint32_t tick;                      ///< System timestamp in milliseconds
    float accX, accY, accZ;            ///< High-G acceleration data
};

/**
 * @brief GNSS/GPS data structure
 *
 * Complete GPS positioning and timing information.
 * Provides location, altitude, speed, and precise timing data.
 */
struct GNSSData {
    uint32_t tick;              ///< System timestamp in milliseconds
    float latitude;             ///< Latitude in decimal degrees
    float longitude;            ///< Longitude in decimal degrees
    float altitude;             ///< Altitude above sea level in meters
    uint8_t satelliteCount;     ///< Number of satellites in view
    bool fix;                   ///< GPS fix status (true = valid fix)
    uint8_t hour;               ///< UTC hour (0-23)
    uint8_t minute;             ///< UTC minute (0-59)
    uint8_t second;             ///< UTC second (0-59)
    uint16_t millisecond;       ///< UTC millisecond (0-999)
    float speed;                ///< Ground speed in m/s
    float course;               ///< Course over ground in degrees
};

/**
 * @brief Flight logic data structure
 *
 * Contains processed flight data and current flight state.
 * Represents the output of flight logic algorithms.
 */
struct FlightLogicData {
    uint32_t tick;                  ///< System timestamp in milliseconds
    uint8_t flightState;           ///< Current flight state (enum value)
    float filtered_altitude;        ///< Kalman-filtered altitude in meters
    float filtered_acceleration;    ///< Filtered acceleration in G-forces
};

//struct BatLevelData {
//	uint32_t tick;
//	uint16_t batLevel;
//};

struct TemperatureData {
	uint32_t tick;
	float temperature;
};

struct TelemetryBundle {
    uint32_t tick;

    // MS5607
    int16_t  temperature;
    int32_t  pressure;

    // ICM-45686 accel (g)
    float accel_x;
    float accel_y;
    float accel_z;

    // ICM-45686 gyro (dps)
    float gyro_x;
    float gyro_y;
    float gyro_z;

    // ADXL375 high-G accel (g)
    float highg_x;
    float highg_y;
    float highg_z;

    // Flight logic
    uint8_t  flightState;
    float    filtered_altitude;
    float    filtered_acceleration;
};

#endif /* FILELOGSTRUCT_H_ */
