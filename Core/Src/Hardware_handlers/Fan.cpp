/*
 * Fan.cpp
 *
 *  Created on: Oct 29, 2023
 *      Author: janok
 */

#include "Fan.h"

Fan::Fan(GPIO_TypeDef* fan_port, uint16_t fan_pin) {
	this->fan_port = fan_port;
	this->fan_pin = fan_pin;

	this->timer = nullptr;
	this->timer_channel = 0;
	this->duty_cycle = 0.0f;
}

Fan::Fan(GPIO_TypeDef* fan_port, uint16_t fan_pin, TIM_HandleTypeDef* timer, uint32_t timer_channel) {
	this->fan_port = fan_port;
	this->fan_pin = fan_pin;

	this->timer = timer;
	this->timer_channel = timer_channel;
	this->duty_cycle = 0.0f;
}

Fan::~Fan() {

}

void Fan::set_duty_cycle(float duty_cycle) {
	this->duty_cycle = duty_cycle;
	uint32_t timer_period = this->timer->Init.Period;
	uint32_t ccr_value = (uint32_t)((duty_cycle / 255.0f) * timer_period);

	if (this->timer_channel == TIM_CHANNEL_1) this->timer->Instance->CCR1 = ccr_value;
	else if (this->timer_channel == TIM_CHANNEL_2) this->timer->Instance->CCR2 = ccr_value;
	else if (this->timer_channel == TIM_CHANNEL_3) this->timer->Instance->CCR3 = ccr_value;
	else if (this->timer_channel == TIM_CHANNEL_4) this->timer->Instance->CCR4 = ccr_value;
	else if (this->timer_channel == TIM_CHANNEL_5) this->timer->Instance->CCR5 = ccr_value;
	else if (this->timer_channel == TIM_CHANNEL_6) this->timer->Instance->CCR6 = ccr_value;
}

void Fan::start_fan_timer() {
	HAL_TIM_Base_Init(this->timer);
	HAL_TIM_Base_Start_IT(this->timer);
	HAL_TIM_PWM_Start_IT(this->timer, this->timer_channel);
}

void Fan::stop_fan_timer() {
	HAL_TIM_Base_Stop_IT(this->timer);
	HAL_TIM_PWM_Stop_IT(this->timer, this->timer_channel);
	turn_off_fan();
}

void Fan::fan_timer_callback() {
	turn_on_fan();
}

void Fan::fan_pwm_callback() {
	turn_off_fan();
}

void Fan::turn_on_fan() {
	HAL_GPIO_WritePin(fan_port, fan_pin, GPIO_PIN_SET);
}

void Fan::turn_off_fan() {
	HAL_GPIO_WritePin(fan_port, fan_pin, GPIO_PIN_RESET);
}

TIM_HandleTypeDef* Fan::get_timer() {
	return this->timer;
}
