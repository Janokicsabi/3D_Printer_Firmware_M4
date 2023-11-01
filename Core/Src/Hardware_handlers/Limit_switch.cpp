/*
 * Limitswitch.cpp
 *
 *  Created on: Oct 26, 2023
 *      Author: janok
 */

#include <Limit_switch.h>


Limit_switch::Limit_switch(GPIO_TypeDef* switch_port = nullptr, uint32_t switch_pin = 0, Motor* motor = nullptr) {
	this->switch_port = switch_port;
	this->switch_pin = switch_pin;
	this->motor = motor;
}

Limit_switch::~Limit_switch() {

}

bool Limit_switch::is_switch_pressed() {
	GPIO_PinState limit_state = HAL_GPIO_ReadPin(this->switch_port, this->switch_pin);
	return (limit_state == GPIO_PIN_RESET ? true : false);		//Reverse logic -> Pressed when RESET
}

void Limit_switch::switch_pressed_callback() {
	motor->stop_motor_timer();
}

