/*
 * Pyro.h
 *
 *  Created on: Jun 8, 2025
 *      Author: Korn
 */

#ifndef PYRO_PYRO_H_
#define PYRO_PYRO_H_

#include "main.h"
#include "cmsis_os2.h"

extern TIM_HandleTypeDef htim3;

// PWM_A
static void firePyroChannel1(){
	HAL_GPIO_WritePin(PYRO_CH1_GPIO_Port, PYRO_CH1_Pin, GPIO_PIN_SET);
	osDelay(1000);
	HAL_GPIO_WritePin(PYRO_CH1_GPIO_Port, PYRO_CH1_Pin, GPIO_PIN_RESET);
}

static void firePyroChannel2(){
	HAL_GPIO_WritePin(PYRO_CH2_GPIO_Port, PYRO_CH2_Pin, GPIO_PIN_SET);
	osDelay(1000);
	HAL_GPIO_WritePin(PYRO_CH2_GPIO_Port, PYRO_CH2_Pin, GPIO_PIN_RESET);
}


void setServo1Degree(uint32_t degree){
	htim3.Instance->CCR3 = 25 + (degree / 180.0) * 100.0;
}

void setServo2Degree(uint32_t degree){
	htim3.Instance->CCR4 = 25 + (degree / 180.0) * 100.0;
}


static void setupDodo(){
	// move to 68 and idle
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
	setServo2Degree(68);
	osDelay(2000);
//	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}

// Rotor Leash
//intial position 58 degree hold until main deployment
//engage -> 80 degree (stall for 5 seconds)
//disengage -> 58 degree
static void fireDodo(){
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    // Arm: Move to 0° and stall for 3 seconds
	setServo2Degree(40);
    osDelay(2000);

    // Engage: Move to 180° and stall for 3 seconds
    for(int i = 40; i <= 180; i += 5){
    	setServo2Degree(i);
    	osDelay(1);
    }
    osDelay(2000);

    // Disengage: Return to 68°
    for(int i = 180; i >= 68; i -= 2){
    	setServo2Degree(i);
    	osDelay(1);
    }
    setServo2Degree(68);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
//    osDelay(3000);
}

// Dodo
//initial position 68 degree hold until apogee
//to arm -> 0 degree (stall for 3 seconds)
//engage -> 180 degree (stall for 3 seconds)
//disengage -> 68 degree
static void setupRotorLeash(){
    // Move to 58° and idle
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    setServo1Degree(58);
    osDelay(3000); // Give time to settle
//    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
}

static void fireRotorLeash(){
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    // Engage: Move to 80° and stall for 5 seconds
    setServo1Degree(80);
    osDelay(5000);

    // Disengage: Return to 58°
    setServo1Degree(58);
    osDelay(1000);

    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}


#endif /* PYRO_PYRO_H_ */
