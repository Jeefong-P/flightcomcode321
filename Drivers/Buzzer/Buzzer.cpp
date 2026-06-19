#include "Buzzer.h"

Buzzer::Buzzer(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t tim_freq, void (*delay)(uint32_t)) :
	htim(htim), channel(channel), tim_freq(tim_freq), delay(delay)
{

}

void Buzzer::playFrequency(float freq)
{
	if(freq == 0) {
		HAL_TIM_PWM_Stop(htim, channel);
	}else{
		HAL_TIM_PWM_Start(htim, channel);
		__HAL_TIM_SET_PRESCALER(htim, presForFrequency(freq));
	}
}

void Buzzer::playNote(float freq, uint32_t duration)
{
	playFrequency(freq);
	delay(duration);
	stop();
}

void Buzzer::stop()
{
	playFrequency(0);
}

void Buzzer::shortBeep(){
	shortBeep(defaultFrequency);
}

void Buzzer::longBeep()
{
	longBeep(defaultFrequency);
}

void Buzzer::shortBeep(float freq)
{
	playFrequency(freq);
	delay(100);
	stop();
	delay(10);
}

void Buzzer::longBeep(float freq)
{
	playFrequency(freq);
	delay(100);
	stop();
	delay(100);
}

uint32_t Buzzer::presForFrequency(float frequency)
{
	if (frequency == 0) return 0;
	// I understand 10000 comes from the Counter Period in config
	return (uint32_t) (((float) tim_freq/(10000*frequency))-1);  // 1 is added in the register
}

void Buzzer::NormalOperation()
{
	shortBeep(BUZZER_NOTE_G7);
	shortBeep(BUZZER_NOTE_G7);
	shortBeep(BUZZER_NOTE_G7);
	longBeep(BUZZER_NOTE_E7);
	longBeep(BUZZER_NOTE_E7);
	longBeep(BUZZER_NOTE_E7);
	delay(1000);
}

void Buzzer::SDCardFailure()
{
	longBeep();
	shortBeep();
	delay(1000);
}

void Buzzer::LoRaFailure()
{
	longBeep();
	shortBeep();
	shortBeep();
	delay(1000);
}

void Buzzer::GNSSFailure()
{
	longBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	delay(1000);
}

void Buzzer::MS5607Failure()
{
	longBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	delay(1000);
}

void Buzzer::BMI088Failure()
{
	longBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	delay(1000);
}

void Buzzer::ADXL375Failure()
{
	longBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	shortBeep();
	delay(1000);
}
