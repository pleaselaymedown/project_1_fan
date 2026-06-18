/*
 * led.c
 *
 *  Created on: 2026. 1. 22.
 *      Author: user22
 */

#include "led.h"

LED_CONTROL led[8]={
		{GPIOC, GPIO_PIN_8, GPIO_PIN_SET, GPIO_PIN_RESET},//0
		{GPIOC, GPIO_PIN_6, GPIO_PIN_SET, GPIO_PIN_RESET},//1
		{GPIOC, GPIO_PIN_5, GPIO_PIN_SET, GPIO_PIN_RESET},//2
		{GPIOA, GPIO_PIN_12, GPIO_PIN_SET, GPIO_PIN_RESET},//3
		{GPIOA, GPIO_PIN_11, GPIO_PIN_SET, GPIO_PIN_RESET},//4
		{GPIOB, GPIO_PIN_12, GPIO_PIN_SET, GPIO_PIN_RESET},//5
		{GPIOB, GPIO_PIN_2, GPIO_PIN_SET, GPIO_PIN_RESET},//6
		{GPIOB, GPIO_PIN_1, GPIO_PIN_SET, GPIO_PIN_RESET},//7
};


void ledOn(uint8_t num){
	for(uint8_t i = 0; i < num; i++){ //LED를 num개만큼 켜기
		HAL_GPIO_WritePin(led[i].port, led[i].number, led[i].onState);
	}
}

void ledOnEach(uint8_t num){
		HAL_GPIO_WritePin(led[num].port, led[num].number, led[num].onState);
}

void ledOff(uint8_t num){
	for(uint8_t i = 0; i < num; i++){ //LED를 num개만큼 끄기
			HAL_GPIO_WritePin(led[i].port, led[i].number, led[i].offState);
		}
}
void ledToggle(uint8_t num){
	for(uint8_t i=0; i<num; i++){
		HAL_GPIO_TogglePin(led[i].port, led[i].number);
	}
}

void ledShift(uint8_t num, uint8_t left){
	uint8_t x = 0;
	for(uint8_t i = 0; i < num;i++){ //LED를 num개만큼 켜기
			if(left) x=i;
			else x=num-i-1;
			HAL_GPIO_WritePin(led[x].port, led[x].number, led[x].onState);
			HAL_Delay(100);
		}
	HAL_Delay(500);
	for(uint8_t i = 0; i < num;i++){ //LED를 num개만큼 끄기
				if(left) x=i;
				else x=num-i-1;
				HAL_GPIO_WritePin(led[x].port, led[x].number, led[x].offState);
				HAL_Delay(100);
			}
}

void ledShiftEach(uint8_t num, uint8_t left){

	if(!left){
		for(uint8_t i = 1; i < num;i++){
				GPIO_PinState x = HAL_GPIO_ReadPin(led[i].port, led[i].number);
				HAL_GPIO_WritePin(led[i-1].port,led[i-1].number,x);
		}
		HAL_GPIO_WritePin(led[num-1].port,led[num-1].number,GPIO_PIN_RESET);
	}else{
		for(int8_t i = num-2; i >= 0; i--){
						GPIO_PinState x = HAL_GPIO_ReadPin(led[i].port, led[i].number);
						HAL_GPIO_WritePin(led[i+1].port,led[i+1].number,x);
		}
		HAL_GPIO_WritePin(led[0].port,led[0].number,GPIO_PIN_RESET);
	}
}
