#ifndef GNSSTHREAD_H_
#define GNSSTHREAD_H_

#include "cmsis_os2.h"

// GNSS removed — module not present on this board revision

void GNSSThread(void *argument) {
    for (;;) {
        osDelay(1000);
    }
}

#endif /* GNSSTHREAD_H_ */
