#ifndef LORATHREAD_H_
#define LORATHREAD_H_

#include "cmsis_os2.h"

// LoRa removed — radio not present on this board revision

void LoRaThread(void *argument) {
    for (;;) {
        osDelay(1000);
    }
}

#endif /* LORATHREAD_H_ */
