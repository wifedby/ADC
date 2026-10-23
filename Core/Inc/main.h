#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define LED_Pin                    GPIO_PIN_9
#define LED_GPIO_Port              GPIOA
#define LED_ON_STATE               GPIO_PIN_SET
#define LED_OFF_STATE              GPIO_PIN_RESET

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
