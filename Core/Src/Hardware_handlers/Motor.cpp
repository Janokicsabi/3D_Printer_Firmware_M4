/*
 * Motor.cpp
 *
 *  Created on: Oct 15, 2023
 *      Author: janok
 */

#include "Motor.h"

#include <cmath>


EventGroupHandle_t Motor::event_motor_standing = xEventGroupCreate();

Motor::Motor(GPIO_TypeDef* step_port = NULL, uint16_t step_pin = 0, GPIO_TypeDef* dir_port = NULL, uint16_t dir_pin = 0, TIM_HandleTypeDef* timer = NULL,
		uint32_t timer_channel = 0, float full_step_degree = 1.8, uint32_t microstep_devider = 1, bool is_soft_pwm = false, PWM_type pwm_type = PWM, uint32_t motor_event_bit = 0) {
	this->step_port = step_port;
	this->step_pin = step_pin;
	this->dir_port = dir_port;
	this->dir_pin = dir_pin;
	this->timer = timer;
	this->timer_channel = timer_channel;
	this->remaining_steps = 0;
	this->full_step_degree = full_step_degree;
	this->microstep_devider = microstep_devider;
	this->is_soft_pwm = is_soft_pwm;
	this->pwm_type = pwm_type;

	this->motor_event_bit = motor_event_bit;
	xEventGroupSetBits(event_motor_standing, this->motor_event_bit);

	this->feedrate_accel_time_diff = 0;
	this->change_motor_dir_pin(0);
}

Motor::Motor() {
	this->step_port = NULL;
	this->step_pin = 0;
	this->dir_port = NULL;
	this->dir_pin = 0;
	this->timer = NULL;
	this->timer_channel = 0;
	this->remaining_steps = 0;
	this->microstep_devider = 1;
	this->is_soft_pwm = false;
	this->pwm_type = PWM;
}

Motor::~Motor() {

}


void Motor::motor_move_const(uint32_t step, uint8_t dir) {
	this->remaining_steps = (int32_t)step;
	change_motor_dir_pin(dir);
	this->feedrate_accel_time_diff = 0;
	this->start_motor_timer();
}

void Motor::set_motor_speed(float move_speed, float one_step_displacement) {
	this->motor_speed = move_speed;
	uint32_t timer_period = calculate_motor_timer_period_from_speed(move_speed, one_step_displacement);
	set_motor_timer_period(timer_period);
}

void Motor::motor_move_accel(uint32_t step, uint8_t dir, uint32_t time_diff) {
	GPIO_PinState limitState;
	limitState = HAL_GPIO_ReadPin(LIMIT_Z_GPIO_Port, LIMIT_Z_Pin);
	if (limitState == GPIO_PIN_RESET  && dir == 0) {
		HAL_GPIO_WritePin(EN_Motor_GPIO_Port, EN_Motor_Pin, GPIO_PIN_SET);
		return;
	}
	this->remaining_steps = step;
	change_motor_dir_pin(dir);
	this->feedrate_accel_time_diff = time_diff;
	this->start_motor_timer();
}

void Motor::change_motor_dir_pin(uint8_t new_dir) {
	if (new_dir == this->move_dir) {
		return;
	}

	this->move_dir = new_dir;
	if (new_dir == 0) {
		HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
	}
}

//@brief	This function calculates the required time delay between steps,
//			so the printer can move continuously with the required speed
//@param	move_speed	The required travelling speed [mm/min]
//TODO: Nem itt kellene a prescaler-t állítani, valahogy kiszervez függvénybe vagy ilyenek
const uint32_t Motor::calculate_motor_timer_period_from_speed(uint32_t move_speed, float one_step_displacement) {
	uint32_t timer_clk_freq = HAL_RCC_GetPCLK2Freq();	//[Hz]
	uint32_t maxTimerValue = this->timer->Instance->ARR;
	uint32_t motor_timer_period = 0;
	uint32_t current_prescaler;
	this->timer->Init.Prescaler = 1;

	current_prescaler = this->timer->Init.Prescaler;
	float timer_freq = (float)timer_clk_freq / ((float)(current_prescaler + 1));
	motor_timer_period = (uint32_t)round(((timer_freq * (float)SECONDS_IN_A_MINUTE * one_step_displacement) / (float)move_speed));

	if (motor_timer_period > maxTimerValue) {
		float prescaler_diff = ceil(motor_timer_period / maxTimerValue);
		this->timer->Init.Prescaler = (current_prescaler + 1) * prescaler_diff;
		timer_freq = (float)timer_clk_freq / ((float)(this->timer->Init.Prescaler + 1));
		motor_timer_period = (uint32_t)round(((timer_freq * (float)SECONDS_IN_A_MINUTE * one_step_displacement) / (float)move_speed));
	}
	return motor_timer_period;
}

//TODO: Ahol nincs hardware PWM ott szoftveres kell
//TODO: Megnézni, hogy ez jól frissíti-e a Timer period-ot. Ha nem, akkor USER_CONSTANT kell!!!
//Ha nem jó a CCR1 beállítás, akkor: __HAL_TIM_SET_COMPARE();
void Motor::set_motor_timer_period(uint32_t timer_period) {
	this->timer->Init.Period = timer_period;

	uint32_t auto_reload_value = (uint32_t)(timer_period / 2.0);
	if (this->timer_channel == TIM_CHANNEL_1) {
		this->timer->Instance->CCR1 = auto_reload_value;
	}
	else if (this->timer_channel == TIM_CHANNEL_2) {
		this->timer->Instance->CCR2 = auto_reload_value;
	}
	else if (this->timer_channel == TIM_CHANNEL_3) {
		this->timer->Instance->CCR3 = auto_reload_value;
	}
	else if (this->timer_channel == TIM_CHANNEL_4) {
		this->timer->Instance->CCR4 = auto_reload_value;
	}
	else if (this->timer_channel == TIM_CHANNEL_5) {
		this->timer->Instance->CCR5 = auto_reload_value;
	}
	else if (this->timer_channel == TIM_CHANNEL_6) {
		this->timer->Instance->CCR6 = auto_reload_value;
	}
}

void Motor::start_motor_timer() {
	HAL_TIM_Base_Init(this->timer);
	HAL_TIM_Base_Start_IT(this->timer);
	if (this->pwm_type == PWM) {
		HAL_TIM_PWM_Start_IT(this->timer, this->timer_channel);
	} else if (this->pwm_type == PWM_N) {
		HAL_TIMEx_PWMN_Start_IT(this->timer, this->timer_channel);
	}
	xEventGroupClearBits(event_motor_standing, this->motor_event_bit);
}

void Motor::stop_motor_timer() {
	HAL_TIM_Base_Stop_IT(this->timer);
	if (this->pwm_type == PWM) {
		HAL_TIM_PWM_Stop_IT(this->timer, this->timer_channel);
	} else if (this->pwm_type == PWM_N) {
		HAL_TIMEx_PWMN_Stop_IT(this->timer, this->timer_channel);
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (xEventGroupSetBitsFromISR(event_motor_standing, this->event_motor_standing, &xHigherPriorityTaskWoken) != pdFAIL) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

void Motor::motor_PWM_callback() {
	HAL_GPIO_WritePin(step_port, step_pin, GPIO_PIN_RESET);
}

void Motor::motor_timer_callback() {
	/*if (this->feedrate_accel_time_diff != 0) {
		this->set_motor_timer_period(this->timer->Init.Period + this->feedrate_accel_time_diff);
	}*/

	if (this->is_soft_pwm) {
		HAL_GPIO_WritePin(step_port, step_pin, GPIO_PIN_SET);
	}

	this->remaining_steps--;
	if (this->remaining_steps <= 0) {
		this->stop_motor_timer();
	}
}

const uint32_t Motor::get_microstep_devider() {
	return this->microstep_devider;
}

TIM_HandleTypeDef* Motor::get_timer() {
	return this->timer;
}

const bool Motor::get_is_soft_pwm() {
	return this->is_soft_pwm;
}

const float Motor::get_full_step_degree() {
	return this->full_step_degree;
}

const bool Motor::is_motor_moving() {
	EventBits_t motor_status = xEventGroupGetBits(event_motor_standing);
	if ((motor_status & this->motor_event_bit) != 0) {
		return false;
	}
	return true;
}

GPIO_TypeDef* Motor::get_step_port() {
	return this->step_port;
}

uint16_t Motor::get_step_pin() {
	return this->step_pin;
}

void Motor::enable_motors() {
	HAL_GPIO_WritePin(EN_Motor_GPIO_Port, EN_Motor_Pin, GPIO_PIN_RESET);
}

void Motor::disable_motors() {
	HAL_GPIO_WritePin(EN_Motor_GPIO_Port, EN_Motor_Pin, GPIO_PIN_SET);
}

