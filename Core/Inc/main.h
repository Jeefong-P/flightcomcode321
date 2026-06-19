/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_SDMMC2_SD_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PYRO_CH1_Pin GPIO_PIN_3
#define PYRO_CH1_GPIO_Port GPIOE
#define PYRO_CH2_Pin GPIO_PIN_4
#define PYRO_CH2_GPIO_Port GPIOE
#define MS5607_MOSI_Pin GPIO_PIN_3
#define MS5607_MOSI_GPIO_Port GPIOC
#define GNSS_RX_Pin GPIO_PIN_0
#define GNSS_RX_GPIO_Port GPIOA
#define GNSS_TX_Pin GPIO_PIN_1
#define GNSS_TX_GPIO_Port GPIOA
#define GNSS_NRST_Pin GPIO_PIN_2
#define GNSS_NRST_GPIO_Port GPIOA
#define LORA_CS_Pin GPIO_PIN_4
#define LORA_CS_GPIO_Port GPIOA
#define LORA_SCK_Pin GPIO_PIN_5
#define LORA_SCK_GPIO_Port GPIOA
#define LORA_MISO_Pin GPIO_PIN_6
#define LORA_MISO_GPIO_Port GPIOA
#define LORA_MOSI_Pin GPIO_PIN_7
#define LORA_MOSI_GPIO_Port GPIOA
#define COMPANION_TX_Pin GPIO_PIN_4
#define COMPANION_TX_GPIO_Port GPIOC
#define COMPANION_RX_Pin GPIO_PIN_5
#define COMPANION_RX_GPIO_Port GPIOC
#define GREEN_LED_Pin GPIO_PIN_0
#define GREEN_LED_GPIO_Port GPIOB
#define LORA_RESET_Pin GPIO_PIN_1
#define LORA_RESET_GPIO_Port GPIOB
#define LORA_BUSY_Pin GPIO_PIN_13
#define LORA_BUSY_GPIO_Port GPIOB
#define BMI088_GYRO_INT4_Pin GPIO_PIN_8
#define BMI088_GYRO_INT4_GPIO_Port GPIOC
#define BMI088_GYRO_INT3_Pin GPIO_PIN_9
#define BMI088_GYRO_INT3_GPIO_Port GPIOC
#define MS5607_CS_Pin GPIO_PIN_8
#define MS5607_CS_GPIO_Port GPIOA
#define MS5607_SCK_Pin GPIO_PIN_9
#define MS5607_SCK_GPIO_Port GPIOA
#define LORA_DIO1_Pin GPIO_PIN_10
#define LORA_DIO1_GPIO_Port GPIOA
#define BMI088_ACC_INT1_Pin GPIO_PIN_0
#define BMI088_ACC_INT1_GPIO_Port GPIOD
#define BMI088_ACC_INT2_Pin GPIO_PIN_1
#define BMI088_ACC_INT2_GPIO_Port GPIOD
#define BMI088_GYRO_CS_Pin GPIO_PIN_2
#define BMI088_GYRO_CS_GPIO_Port GPIOD
#define MS5607_MISO_Pin GPIO_PIN_3
#define MS5607_MISO_GPIO_Port GPIOD
#define ADXL_INT2_Pin GPIO_PIN_4
#define ADXL_INT2_GPIO_Port GPIOD
#define ADXL_INT1_Pin GPIO_PIN_5
#define ADXL_INT1_GPIO_Port GPIOD
#define ADXL_INT1_EXTI_IRQn EXTI5_IRQn
#define BMI088_ACC_CS_Pin GPIO_PIN_6
#define BMI088_ACC_CS_GPIO_Port GPIOD
#define ADXL_CS_Pin GPIO_PIN_0
#define ADXL_CS_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
