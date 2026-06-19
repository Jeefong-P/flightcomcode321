#ifndef SYSTEM_THREAD
#define SYSTEM_THREAD

#include "Buzzer.h"
#include "ThreadInclude.h"


// External hardware timer for buzzer PWM
extern TIM_HandleTypeDef htim17;
extern ADC_HandleTypeDef hadc1;

// Buzzer configuration with lambda delay function
auto delayFunction = [](uint32_t ms) { osDelay(ms); };
Buzzer buzzer(&htim17, TIM_CHANNEL_1, 160000000, delayFunction);

/**
 * @brief Green LED status indicator thread
 *
 * Provides visual feedback of system operation through LED blinking.
 * Asymmetric blink pattern (62ms on, 63ms off) for easy identification.
 *
 * @param argument Unused thread parameter
 */
void GreenLedThread(void *argument) {
    for (;;) {
        // Turn LED on for 62ms
        HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
        osDelay(62);

        // Turn LED off for 63ms
        HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
        osDelay(63);
    }
}

/**
 * @brief Buzzer audio feedback thread
 *
 * Provides audio status indication based on system state:
 * - Normal operation: Success tone when all systems ready
 * - Error conditions: Specific error tones for failed components
 *
 * Only operates when flight state is IDLE to avoid interference
 * during active flight operations.
 *
 * @param argument Unused thread parameter
 */
void BuzzerThread(void *argument) {
    // Initialize buzzer hardware
    osDelay(1000);  // Initial delay for system stabilization

    for (;;) {
        // Check system status and provide appropriate audio feedback
        if (initStatusAllReady()) {
            // All systems operational - play success tone
            buzzer.NormalOperation();
        } else {
            // One or more systems failed - play specific error tones
            if (!initStatus.SDCardReady) {
                buzzer.SDCardFailure();
            }
            if (!initStatus.ms5607Ready) {
                buzzer.MS5607Failure();
            }
            if (!initStatus.icmReady) {
                buzzer.BMI088Failure();
            }
//            if (!initStatus.adxl375Ready) {
//                buzzer.ADXL375Failure();
//            }
        }

        // Wait 10 seconds before next status check
        // Only operate when in IDLE state to avoid interference during flight
        do {
            osDelay(10000);
        } while (flightState != IDLE);
    }
}

//#define VOLT_DIV_R1 10000.0f  // 10k Ohm
//#define VOLT_DIV_R2 2200.0f   // 2.2k Ohm
//
//// 12-bit ADC -> max value = 4095, Vref = 3.3V
//#define ADC_MAX_VALUE 16383.0f
//#define VREF 3300.0f
//
//uint16_t batteryVoltage = 0; // in millivolts
//uint16_t adcValue = 0;
//
//void BatLevelThread(void *argument) {
//
//for(;;) {
//
//    	HAL_ADC_Start(&hadc1);  // Start ADC once before loop
//        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
//        adcValue = HAL_ADC_GetValue(&hadc1);
//        HAL_ADC_Stop(&hadc1);
//
//        // Convert ADC value to actual battery voltage in mV
//        float vAdc = (adcValue * VREF) / ADC_MAX_VALUE;
//        batteryVoltage = (uint16_t) ( vAdc * ((VOLT_DIV_R1 + VOLT_DIV_R2) / VOLT_DIV_R2) );
//        BatLevelData data = {
//            .tick = HAL_GetTick(),
//            .batLevel = batteryVoltage
//        };
//
//        // batLevelLogger.logStruct(&data, sizeof(BatLevelData));
//        osDelay(100);  // 10 seconds
//    }
//}

void BatLevelThread(void *argument){
    for(;;){
        osDelay(1000);
    }
}
#endif /* SYSTEM_THREAD */
