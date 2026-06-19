/*
   MS5607-02 SPI library for ARM STM32F103xx Microcontrollers - C++ Class Implementation
   Based on original work by Joao Pedro Vilas <joaopedrovbs@gmail.com>
*/
/* ============================================================================================
 MS5607-02 device SPI library code for ARM STM32F103xx is placed under the MIT license
Copyright (c) 2020 João Pedro Vilas Boas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 ================================================================================================
 */

#ifndef _MS5607_H_
#define _MS5607_H_

#include "main.h"

/* MS5607 SPI COMMANDS */
#define RESET_COMMAND                 0x1E
#define PROM_READ(address)            (0xA0 | ((address) << 1))         // Macro to change values for the 8 PROM addresses
#define CONVERT_D1_COMMAND            0x40
#define CONVERT_D2_COMMAND            0x50
#define READ_ADC_COMMAND              0x00

/* MS5607 System States Enumeration*/
typedef enum MS5607States {
  MS5607_STATE_FAILED,
  MS5607_STATE_READY
} MS5607StateTypeDef;

/* MS5607 Oversampling Ratio Enumeration*/
typedef enum OSRFactors {
  OSR_256,
  OSR_512 = 0x02,
  OSR_1024 = 0x04,
  OSR_2048 = 0x06,
  OSR_4096 = 0x08
} MS5607OSRFactors;

class MS5607 {
public:
  /**
   * @brief  Constructor for MS5607 class
   */
  MS5607(SPI_HandleTypeDef *hspi_instance, GPIO_TypeDef *gpio_port, uint16_t gpio_pin);

  /**
   * @brief  Initializes MS5607 Sensor
   * @param  SPI Handle address
   * @param  GPIO Port Definition
   * @param  GPIO Pin
   * @retval Initialization status:
   *           - 0 or MS5607_STATE_FAILED: Was not abe to communicate with sensor
   *           - 1 or MS5607_STATE_READY: Sensor initialized OK and ready to use
   */
  MS5607StateTypeDef init();

  /**
   * @brief  Updates the readings from the sensor
   * @note   This function must be called each time you want new values from the sensor
   * @param  None
   * @retval None
   */
  void update();

  /**
   * @brief  Gets the temperature reading from the last sensor update
   * @note   This function must be called after an update()
   * @param  None
   * @retval Temperature in celcius * 100
   */
  int16_t getTemperatureC();

  /**
   * @brief  Gets the pressure reading from the last sensor update
   * @note   This function must be called after an update()
   * @param  None
   * @retval Pressure in Pascal
   */
  int32_t getPressurePa();

  /**************************************************************************/
  /*!
      @brief Calculates the altitude (in meters).

      Reads the current atmostpheric pressure (in hPa) from the sensor and
     calculates via the provided sea-level pressure (in hPa).

      @param  seaLevelPressure      Sea-level pressure in hPa
      @return Altitude in meters
  */
  /**************************************************************************/
  float getAltitude();

  /**
   * @brief  Sets the temperature reading oversamplig ratio
   * @param  OSR factor from 256 to 4096
   * @retval None
   */
  void setTemperatureOSR(MS5607OSRFactors osr);

  /**
   * @brief  Sets the pressure reading oversamplig ratio
   * @param  OSR factor from 256 to 4096
   * @retval None
   */
  void setPressureOSR(MS5607OSRFactors osr);

  void setDelayFunction(void (*delay)(uint32_t));

  void setReferenceLevelPressure(float pressure);

private:
  /* MS5607 PROM Data Structure */
  struct PromData {
    uint16_t reserved;
    uint16_t sens;
    uint16_t off;
    uint16_t tcs;
    uint16_t tco;
    uint16_t tref;
    uint16_t tempsens;
    uint16_t crc;
  };

  /* Uncompensated digital Values */
  struct UncompensatedValues {
    uint32_t pressure;
    uint32_t temperature;
  };

  /* Actual readings */
  struct Readings {
	int32_t pressure; // mbar (1 atm = 1013.25 mbar)
    int32_t temperature; // *C
  };

  /**
   * @brief  Enables the chip select pin
   * @param  None
   * @retval None
   */
  void enableCSB();

  /**
   * @brief  Disables the chip select pin
   * @param  None
   * @retval None
   */
  void disableCSB();

  /**
   * @brief  Reads MS5607 PROM Content
   * @note   Must be called only on device initialization
   * @param  None
   * @retval None
   */
  void readProm();

  /**
   * @brief  Reads uncompensated content from the MS5607 ADC
   * @note   Must be called before every convertion
   * @param  None
   * @retval None
   */
  void readUncompensatedValues();

  /**
   * @brief  Converts uncompensated values into real world values
   * @note   Must be called after readUncompensatedValues
   * @param  None
   * @retval None
   */
  void convertValues();

  /* Private SPI Handler */
  SPI_HandleTypeDef *hspi;

  /* Private GPIO CS Pin Variables */
  GPIO_TypeDef *CS_GPIO_Port;
  uint16_t CS_Pin;

  /* SPI Transmission Data */
  uint8_t SPITransmitData;

  /* Private OSR Instantiations */
  uint8_t pressureOSR;
  uint8_t temperatureOSR;

  /* Private data holders */
  PromData promData;
  UncompensatedValues uncompValues;
  Readings readings;

  float referenceLevelPressure;

  void (*delay)(uint32_t);
};

#endif /* _MS5607_H_ */
