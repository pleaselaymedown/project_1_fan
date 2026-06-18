/*
 * led.h
 *
 *  Created on: 2026. 1. 22.
 *      Author: user22
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

typedef struct{
	GPIO_TypeDef *port;
	uint16_t	number;
	GPIO_PinState onState;
	GPIO_PinState offState;
}LED_CONTROL;

void ledOnEach(uint8_t num);
void ledOn(uint8_t num);
void ledOff(uint8_t num);
void ledToggle(uint8_t num);

void ledShift(uint8_t num, uint8_t left);
void ledShiftEach(uint8_t num, uint8_t left);

#ifdef __cplusplus
}
#endif

#endif /* INC_LED_H_ */
