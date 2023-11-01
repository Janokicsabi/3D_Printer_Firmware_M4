/*
 * Fan.cpp
 *
 *  Created on: Oct 29, 2023
 *      Author: janok
 */

#include "Fan.h"

Fan::Fan(GPIO_TypeDef* fan_port = nullptr, uint16_t fan_pin = 0) {
	this->fan_port = fan_port;
	this->fan_pin = fan_pin;
}

Fan::~Fan() {

}

void Fan::turn_on_fan() {
	HAL_GPIO_WritePin(fan_port, fan_pin, GPIO_PIN_SET);
}

void Fan::turn_off_fan() {
	HAL_GPIO_WritePin(fan_port, fan_pin, GPIO_PIN_RESET);
}
