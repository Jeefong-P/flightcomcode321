/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ThreadInclude.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for LoRaTask */
osThreadId_t LoRaTaskHandle;
const osThreadAttr_t LoRaTask_attributes = {
  .name = "LoRaTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for FlightLogicTask */
osThreadId_t FlightLogicTaskHandle;
const osThreadAttr_t FlightLogicTask_attributes = {
  .name = "FlightLogicTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for BMI088Task */
osThreadId_t BMI088TaskHandle;
const osThreadAttr_t BMI088Task_attributes = {
  .name = "BMI088Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 1024 * 4
};
/* Definitions for MS5607Task */
osThreadId_t MS5607TaskHandle;
const osThreadAttr_t MS5607Task_attributes = {
  .name = "MS5607Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for UARTTask */
osThreadId_t UARTTaskHandle;
const osThreadAttr_t UARTTask_attributes = {
  .name = "UARTTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for SDCardTask */
osThreadId_t SDCardTaskHandle;
const osThreadAttr_t SDCardTask_attributes = {
  .name = "SDCardTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 2048 * 4
};
/* Definitions for GreenLedTask */
osThreadId_t GreenLedTaskHandle;
const osThreadAttr_t GreenLedTask_attributes = {
  .name = "GreenLedTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for BuzzerTask */
osThreadId_t BuzzerTaskHandle;
const osThreadAttr_t BuzzerTask_attributes = {
  .name = "BuzzerTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for FlightLogicLogTask */
osThreadId_t FlightLogicLogTaskHandle;
const osThreadAttr_t FlightLogicLogTask_attributes = {
  .name = "FlightLogicLogTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for IMULogTask */
osThreadId_t IMULogTaskHandle;
const osThreadAttr_t IMULogTask_attributes = {
  .name = "IMULogTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for BatLevelTask */
osThreadId_t BatLevelTaskHandle;
const osThreadAttr_t BatLevelTask_attributes = {
  .name = "BatLevelTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of LoRaTask */
  LoRaTaskHandle = osThreadNew(LoRaThread, NULL, &LoRaTask_attributes);

  /* creation of FlightLogicTask */
  FlightLogicTaskHandle = osThreadNew(FlightLogicThread, NULL, &FlightLogicTask_attributes);

  /* creation of BMI088Task */
  BMI088TaskHandle = osThreadNew(IMUThread, NULL, &BMI088Task_attributes);

  /* creation of MS5607Task */
  MS5607TaskHandle = osThreadNew(MS5607Thread, NULL, &MS5607Task_attributes);

  /* creation of UARTTask */
  UARTTaskHandle = osThreadNew(UARTThread, NULL, &UARTTask_attributes);

  /* creation of SDCardTask */
  SDCardTaskHandle = osThreadNew(SDCardThread, NULL, &SDCardTask_attributes);

  /* creation of GreenLedTask */
  GreenLedTaskHandle = osThreadNew(GreenLedThread, NULL, &GreenLedTask_attributes);

  /* creation of BuzzerTask */
  BuzzerTaskHandle = osThreadNew(BuzzerThread, NULL, &BuzzerTask_attributes);

  /* creation of FlightLogicLogTask */
  FlightLogicLogTaskHandle = osThreadNew(FlightLogicLogThread, NULL, &FlightLogicLogTask_attributes);

  /* creation of IMULogTask */
  IMULogTaskHandle = osThreadNew(IMULogThread, NULL, &IMULogTask_attributes);

  /* creation of BatLevelTask */
  BatLevelTaskHandle = osThreadNew(BatLevelThread, NULL, &BatLevelTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

