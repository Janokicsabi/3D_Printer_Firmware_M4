/*
 * temperature_control.h
 *
 *  Created on: Sep 27, 2023
 *      Author: janok
 */

#ifndef INC_TEMPERATURE_CONTROL_H_
#define INC_TEMPERATURE_CONTROL_H_

#include "main.h"


void task_temp_control(void* param);

float compute_foxboro_controller(const float setpoint, const float measured_value, float* fox);
void task_temp_log(void* param);
void set_duty_cycle(float new_duty_cycle);
float get_duty_cycle();
void set_goal_temp(float new_goal_temp);
float get_goal_temp();
void init_temperature_control(TIM_HandleTypeDef* htim, uint32_t channel);
void modify_timer_compare(TIM_HandleTypeDef* htim, uint32_t channel, const float duty_cycle);
uint32_t calculate_compare_num(TIM_HandleTypeDef* htim, uint32_t dutyCycle);
void start_temperature_control(TIM_HandleTypeDef* htim, uint32_t , ADC_HandleTypeDef* adc, uint32_t dutyCycle);
void get_temperature_callback();
void heater_callback();
void heater_turn_on();
void heater_turn_off();
void heater_emergency_stop();

#endif /* INC_TEMPERATURE_CONTROL_H_ */
