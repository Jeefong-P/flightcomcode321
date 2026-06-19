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

#include "MS5607.h"
#include <cstring>
#include <cmath>

// Constructor implementation
MS5607::MS5607(SPI_HandleTypeDef *hspi_instance, GPIO_TypeDef *gpio_port, uint16_t gpio_pin) :
                    pressureOSR(OSR_256), temperatureOSR(OSR_256), referenceLevelPressure(1013.25) {
// Store hardware interface information

  hspi = hspi_instance;
  CS_GPIO_Port = gpio_port;
  CS_Pin = gpio_pin;

  this->delay = [](uint32_t ms) {HAL_Delay(ms);};
// Initialize values to 0
  SPITransmitData = 0;
  memset(&promData, 0, sizeof(PromData));
  memset(&uncompValues, 0, sizeof(UncompensatedValues));
  memset(&readings, 0, sizeof(Readings));
}

MS5607StateTypeDef MS5607::init() {
  // Reset the device
  enableCSB();
  SPITransmitData = RESET_COMMAND;
  HAL_SPI_Transmit(hspi, &SPITransmitData, 1, 10);
  HAL_Delay(3);
  disableCSB();

  // Read the PROM data
  readProm();

  // Verify the connection by checking PROM reserved value
  if (promData.reserved == 0x00 || promData.reserved == 0xff)
    return MS5607_STATE_FAILED;
  else
    return MS5607_STATE_READY;
}

void MS5607::readProm() {
  uint8_t address;
  uint16_t *structPointer;

  // As the PROM is made of 8 16bit addresses I used a pointer for accessing the data structure
  structPointer = (uint16_t *)&promData;

  for (address = 0; address < 8; address++) {
    SPITransmitData = PROM_READ(address);
    enableCSB();
    HAL_SPI_Transmit(hspi, &SPITransmitData, 1, 10);
    // Receive two bytes at once and stores it directly at the structure
    HAL_SPI_Receive(hspi, (uint8_t *)structPointer, 2, 10);
    disableCSB();
    structPointer++;
  }

  // Byte swap on 16bit integers
  structPointer = (uint16_t *)&promData;
  for (address = 0; address < 8; address++) {
    uint8_t *toSwap = (uint8_t *)structPointer;
    uint8_t secondByte = toSwap[0];
    toSwap[0] = toSwap[1];
    toSwap[1] = secondByte;
    structPointer++;
  }
}

void MS5607::readUncompensatedValues() {
  // Sensor reply data buffer
  uint8_t reply[3];

  // Read pressure data first
  enableCSB();
  // Assemble the conversion command based on previously set OSR
  SPITransmitData = CONVERT_D1_COMMAND | pressureOSR;
  HAL_SPI_Transmit(hspi, &SPITransmitData, 1, 10);

  // Wait appropriate time based on OSR setting
  if (pressureOSR == 0x00)
    HAL_Delay(1);
  else if (pressureOSR == 0x02)
    HAL_Delay(2);
  else if (pressureOSR == 0x04)
    HAL_Delay(3);
  else if (pressureOSR == 0x06)
    HAL_Delay(5);
  else
    HAL_Delay(10);

  disableCSB();

  // Performs the reading of the 24 bits from the ADC
  enableCSB();
  SPITransmitData = READ_ADC_COMMAND;
  HAL_SPI_Transmit(hspi, &SPITransmitData, 1, 10);
  HAL_SPI_Receive(hspi, reply, 3, 10);
  disableCSB();

  // Transfer the 24bits read into a 32bit int
  uncompValues.pressure = ((uint32_t)reply[0] << 16) | ((uint32_t)reply[1] << 8) | (uint32_t)reply[2];

  // Now read temperature data
  enableCSB();
  // Assemble the conversion command based on previously set OSR
  SPITransmitData = CONVERT_D2_COMMAND | temperatureOSR;
  HAL_SPI_Transmit(hspi, &SPITransmitData, 1, 10);

  // Wait appropriate time based on OSR setting
  if (temperatureOSR == 0x00)
    HAL_Delay(1);
  else if (temperatureOSR == 0x02)
    HAL_Delay(2);
  else if (temperatureOSR == 0x04)
    HAL_Delay(3);
  else if (temperatureOSR == 0x06)
    HAL_Delay(5);
  else
    HAL_Delay(10);

  disableCSB();

  enableCSB();
  SPITransmitData = READ_ADC_COMMAND;
  HAL_SPI_Transmit(hspi, &SPITransmitData, 1, 10);
  HAL_SPI_Receive(hspi, reply, 3, 10);
  disableCSB();

  // Assemble the conversion command based on previously set OSR
  uncompValues.temperature = ((uint32_t)reply[0] << 16) | ((uint32_t)reply[1] << 8) | (uint32_t)reply[2];
}

void MS5607::convertValues() {
  int32_t dT;
  int32_t TEMP;
  int64_t OFF;
  int64_t SENS;

  // Calculate temperature difference
  dT = uncompValues.temperature - ((int32_t)(promData.tref << 8));

  // Calculate actual temperature
  TEMP = 2000 + (((int64_t)dT * promData.tempsens) >> 23);

  // Calculate offset and sensitivity at actual temperature
  OFF = ((int64_t)promData.off << 17) + (((int64_t)promData.tco * dT) >> 6);
  SENS = ((int64_t)promData.sens << 16) + (((int64_t)promData.tcs * dT) >> 7);

  // Second order temperature compensation
  if (TEMP < 2000) {
    int32_t T2 = ((int64_t)dT * (int64_t)dT) >> 31;
    int32_t TEMPM = TEMP - 2000;
    int64_t OFF2 = (61 * (int64_t)TEMPM * (int64_t)TEMPM) >> 4;
    int64_t SENS2 = 2 * (int64_t)TEMPM * (int64_t)TEMPM;

    if (TEMP < -1500) {
      int32_t TEMPP = TEMP + 1500;
      int32_t TEMPP2 = TEMPP * TEMPP;
      OFF2 = OFF2 + (int64_t)15 * TEMPP2;
      SENS2 = SENS2 + (int64_t)8 * TEMPP2;
    }

    TEMP -= T2;
    OFF -= OFF2;
    SENS -= SENS2;
  }

  // Calculate compensated pressure
  readings.pressure = ((((int64_t)uncompValues.pressure * SENS) >> 21) - OFF) >> 15;
  readings.temperature = TEMP;
}

void MS5607::update() {
  readUncompensatedValues();
  convertValues();
}

int16_t MS5607::getTemperatureC() {
  return readings.temperature;
}

int32_t MS5607::getPressurePa() {
  return readings.pressure;
}

void MS5607:: setReferenceLevelPressure(float pressure) {
	referenceLevelPressure = pressure;
}

float MS5607::getAltitude() {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude. See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = getPressurePa() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / referenceLevelPressure, 0.1903));
}

void MS5607::enableCSB() {
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

void MS5607::disableCSB() {
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

void MS5607::setTemperatureOSR(MS5607OSRFactors osr) {
  temperatureOSR = osr;
}

void MS5607::setPressureOSR(MS5607OSRFactors osr) {
  pressureOSR = osr;
}

void MS5607::setDelayFunction(void (*delay)(uint32_t)){
	this->delay = delay;
}
