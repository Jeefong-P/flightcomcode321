/*
 * Buzzer.h
 *
 *  Created on: May 30, 2025
 *      Author: Korn-PC
 */

#ifndef BUZZER_BUZZER_H_
#define BUZZER_BUZZER_H_

#include "BuzzerNote.h"
#include "main.h"

class Buzzer {
public:
	Buzzer(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t tim_freq, void (*delay)(uint32_t));
	void playFrequency(float freq);
	void playNote(float freq, uint32_t duration);
	void stop();
	void shortBeep();
	void longBeep();
	void shortBeep(float freq);
	void longBeep(float freq);

	void NormalOperation();
	void SDCardFailure();
	void LoRaFailure();
	void GNSSFailure();
	void MS5607Failure();
	void BMI088Failure();
	void ADXL375Failure();

private:
	uint32_t presForFrequency(float freq);

	TIM_HandleTypeDef *htim;
	uint32_t channel;
	uint32_t tim_freq;
	float defaultFrequency = BUZZER_NOTE_G6;
	void (*delay)(uint32_t);
};


#endif /* BUZZER_BUZZER_H_ */
