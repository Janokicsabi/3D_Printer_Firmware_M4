/*
 * Limitswitch.cpp
 *
 *  Created on: Oct 26, 2023
 *      Author: janok
 */

#include <Limit_switch.h>


Limit_switch::Limit_switch() {
	this->switch_port = nullptr;
	this->switch_pin = 0;
	this->motor = nullptr;
}

Limit_switch::Limit_switch(GPIO_TypeDef* switch_port, uint16_t switch_pin, Stepper* motor) {
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

uint16_t Limit_switch::get_switch_pin() {
	return this->switch_pin;
}

void Limit_switch::switch_pressed_callback() {

}

