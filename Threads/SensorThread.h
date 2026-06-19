/**
 * @file SensorThread.h
 * @brief Sensor data acquisition threads for flight sensors
 * @author Korn
 * @date June 2025
 *
 * This file manages all flight sensors including barometric altimeter,
 * IMU (accelerometer/gyroscope), and high-G accelerometer. Data is
 * continuously sampled and logged to SD card with adaptive rates.
 */

#ifndef SENSORTHREAD_H_
#define SENSORTHREAD_H_

#include "ICM.h"
#include "ADXL375.h"
#include "MS5607.h"
#include "GlobalInclude.h"
#include "FileLogStruct.h"

// External SPI handles
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

// Sensor instances
ADXL375 adxl375;  // High-G accelerometer (±200g)

ICM45686_Handle icm = {0};  // ICM-45686 6-axis IMU handle

MS5607 ms5607(&hspi2, MS5607_CS_GPIO_Port, MS5607_CS_Pin);    // Barometric sensor

/*
 SimpleKalmanFilter(e_mea, e_est, q);
 e_mea: Measurement Uncertainty
 e_est: Estimation Uncertainty
 q: Process Noise
 */
SimpleKalmanFilter altitudeFilter(3, 2, 0.1);
SimpleKalmanFilter accelerationFilter(0.5, 1, 0.05);

float getReferencePressureLevel() {
	uint16_t cnt = 0;
	uint64_t sum = 0;
	uint64_t start_tick = HAL_GetTick();
	while(HAL_GetTick() - start_tick <= 100){
		ms5607.update();
		sum += ms5607.getPressurePa();
		cnt++;
		osDelay(1);
	}
	return (float) (sum / cnt) / 100.0;
}

void logMS5607Data() {
    // Prepare data structure for logging
    MS5607Data data = {
        .tick = HAL_GetTick(),
        .temperature = ms5607.getTemperatureC(),  // Store as °C
        .pressure = ms5607.getPressurePa()                // Store as Pa
    };

    // Log data to SD card
    ms5607Logger.logStruct(&data, sizeof(data));
}

/**
 * @brief MS5607 barometric sensor thread
 * @param argument Thread parameter (unused)
 *
 * Continuously reads pressure and temperature data from MS5607 sensor.
 * Provides altitude calculation for flight state determination.
 */
void MS5607Thread(void *argument) {
    // Initialize MS5607 barometric sensor
    if (ms5607.init() != MS5607_STATE_READY) {
        // Sensor initialization failed - enter safe mode
        while (1) {
            osDelay(1000);
        }
    }

    // Configure delay function for sensor timing
    auto delayFunction = [](uint32_t ms) { osDelay(ms); };
    ms5607.setDelayFunction(delayFunction);

    // Configure oversampling for optimal precision vs speed trade-off
    ms5607.setPressureOSR(OSR_4096);    // High precision for pressure
    ms5607.setTemperatureOSR(OSR_256);  // Medium precision for temperature
    ms5607.setReferenceLevelPressure(getReferencePressureLevel());

    // Signal sensor is ready
    initStatus.ms5607Ready = 1;

    uint64_t lastLog = 0;

    // Main sensor loop
    for (;;) {

    	ms5607.update();
    	uint8_t idleState = flightState == IDLE || flightState == TOUCHDOWN;
//    	if(idleState && (isnan(ms5607.getAltitude()) or fabsf(ms5607.getAltitude()) >= 2.0)){
//    		ms5607.setReferenceLevelPressure(ms5607.getPressurePa() / 100.0);
//    		osDelay(1);
//    		continue;
//    	}
        altitude = altitudeFilter.updateEstimate(ms5607.getAltitude());

    	if(idleState){
    		if(HAL_GetTick() - lastLog >= 1000){
        		logMS5607Data();
        		ms5607.setReferenceLevelPressure(getReferencePressureLevel());
        		lastLog = HAL_GetTick();
    		}
    	}else{
    		logMS5607Data();
    	}

    	osDelay(1);
    }
}

// Interrupt flags for sensor data ready signals
volatile uint8_t icm_acc_int = 0;
volatile uint8_t icm_gyro_int = 0;
volatile uint8_t adxl375_int = 0;

/**
 * @brief GPIO interrupt callback for sensor data ready signals
 * @param GPIO_Pin GPIO pin that triggered interrupt
 */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//    switch (GPIO_Pin) {
//        case BMI088_ACC_CS_Pin:
//            bmi088_acc_int = 1;
//            break;
//        case BMI088_GYRO_CS_Pin:
//            bmi088_gyro_int = 1;
//            break;
//        case ADXL_CS_Pin:
//            adxl375_int = 1;
//            break;
//        default:
//            break;
//    }
//}

void logICMData() {
    ICMAccelerationData accData = {
        .tick = HAL_GetTick(),
        .accX = icm.accel_x / 2048.0f,
        .accY = icm.accel_y / 2048.0f,
        .accZ = icm.accel_z / 2048.0f,
    };

    ICMGyroData gyroData = {
        .tick = HAL_GetTick(),
        .gyX = icm.gyro_x / 16.384f,
        .gyY = icm.gyro_y / 16.384f,
        .gyZ = icm.gyro_z / 16.384f
    };

    icmAccLogger.logStruct(&accData, sizeof(ICMAccelerationData));
    icmGyroLogger.logStruct(&gyroData, sizeof(ICMGyroData));
}

// if new data is same as last, it's probably a read error
bool ICMAccError(ICM45686_Handle& current, ICM45686_Handle& last) {
    return current.accel_x == last.accel_x && current.accel_y == last.accel_y && current.accel_z == last.accel_z;
}

/**
 * @brief IMU and high-G accelerometer thread
 * @param argument Thread parameter (unused)
 *
 * Manages both the ICM-45686 6-axis IMU and ADXL375 high-G accelerometer.
 */
void IMUThread(void *argument) {
    // Initialize ICM-45686 6-axis IMU
    if (ICM45686_Init(&icm, &hspi3, BMI088_ACC_CS_GPIO_Port, BMI088_ACC_CS_Pin) == 1) {
        initStatus.icmReady = 1;
    } else {
        initStatus.icmReady = 0;
    }

    // Initialize ADXL375 high-G accelerometer
    if (adxl375.initialize(&hspi3, ADXL_CS_GPIO_Port, ADXL_CS_Pin) == 0) {
        adxl375.setDataRate(0x0F);      // 3200 Hz sample rate
        adxl375.setPowerMode(ADXL375_POWER_CTL_MEASURE_MODE);     // Enable measurement mode

        initStatus.adxl375Ready = 1;
    } else {
        initStatus.adxl375Ready = 0;
    }

    uint64_t lastIdleLog = 0;
    ICM45686_Handle lastIcm = {0};

    // Main IMU sampling loop
    for (;;) {
        // Process ICM-45686 data if available
        if (initStatus.icmReady) {
            ICM45686_ReadSensors(&icm);

            if (!ICMAccError(icm, lastIcm)) {
                float ax = icm.accel_x / 2048.0f;
                float ay = icm.accel_y / 2048.0f;
                float az = icm.accel_z / 2048.0f;
                float rawAcceleration = sqrtf(ax*ax + ay*ay + az*az);
                lastIcm = icm;
                acceleration = accelerationFilter.updateEstimate(rawAcceleration);
            }
        }

        // Process ADXL375 high-G data if available
        if (initStatus.adxl375Ready) {
        	adxl375.devicePresent();
            if (adxl375.readAcceleration() == HAL_OK) {
                ADXL375Data adxl375Data = {
                    .tick = HAL_GetTick(),
                    .accX = adxl375.getAccX(),
                    .accY = adxl375.getAccY(),
                    .accZ = adxl375.getAccZ()
                };
                 adxl375Logger.logStruct(&adxl375Data, sizeof(ADXL375Data));
            }
        }

        // Adaptive sampling rate
        if (flightState == IDLE || flightState == TOUCHDOWN) {
        	if(HAL_GetTick()-lastIdleLog >= 1000){
        		lastIdleLog = HAL_GetTick();
        		logICMData();
        	}
        } else {
            logICMData();
        }
        osDelay(1);
    }
}

/**
 * @brief IMU data logging thread (currently unused)
 * @note Reserved for future implementation of separate logging thread
 */
void IMULogThread(void *argument) {
    for (;;) {
        osDelay(1000);
    }
}

#endif /* SENSORTHREAD_H_ */
