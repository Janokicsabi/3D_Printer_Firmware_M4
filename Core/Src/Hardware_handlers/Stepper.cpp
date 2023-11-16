/*
 * Stepper.cpp
 *
 *  Created on: Nov 14, 2023
 *      Author: janok
 */

#include "Stepper.h"

#include <cmath>



Stepper::Stepper(GPIO_TypeDef* step_port = NULL, uint16_t step_pin = 0, GPIO_TypeDef* dir_port = NULL, uint16_t dir_pin = 0,
		float full_step_degree = 1.8, uint32_t microstep_devider = 1) {
	this->step_port = step_port;
	this->step_pin = step_pin;
	this->dir_port = dir_port;
	this->dir_pin = dir_pin;
	this->remaining_steps = 0;
	this->full_step_degree = full_step_degree;
	this->microstep_devider = microstep_devider;
	this->change_stepper_dir_pin(0);
}

Stepper::Stepper() {
	this->step_port = NULL;
	this->step_pin = 0;
	this->dir_port = NULL;
	this->dir_pin = 0;
	this->remaining_steps = 0;
	this->microstep_devider = 1;
}

Stepper::~Stepper() {

}

void Stepper::set_goal_speed(float move_speed, float one_step_displacement) {

}

void Stepper::change_stepper_dir_pin(uint8_t new_dir) {
	this->move_dir = new_dir;
	if (new_dir == 0) {
		HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
	}
}

const uint32_t Stepper::get_microstep_devider() {
	return this->microstep_devider;
}

const float Stepper::get_full_step_degree() {
	return this->full_step_degree;
}

GPIO_TypeDef* Stepper::get_step_port() {
	return this->step_port;
}

uint16_t Stepper::get_step_pin() {
	return this->step_pin;
}

void Stepper::enable_steppers() {
	HAL_GPIO_WritePin(EN_Motor_GPIO_Port, EN_Motor_Pin, GPIO_PIN_RESET);
}

void Stepper::disable_steppers() {
	HAL_GPIO_WritePin(EN_Motor_GPIO_Port, EN_Motor_Pin, GPIO_PIN_SET);
}
