/*
 * motor_timer_callback.h
 *
 *  Created on: Oct 24, 2023
 *      Author: janok
 */

#ifndef INC_CPP_CALLBACK_WRAP_H_
#define INC_CPP_CALLBACK_WRAP_H_


#include "main.h"

#ifdef __cplusplus
extern "C" void cpp_wrap_motor_pwm_callback(TIM_HandleTypeDef *htim);
extern "C" void cpp_wrap_motor_timer_callback(TIM_HandleTypeDef *htim);

extern "C" void cpp_wrap_heater_pwm_callback(TIM_HandleTypeDef *htim);
extern "C" void cpp_wrap_heater_timer_callback(TIM_HandleTypeDef *htim);

extern "C" void cpp_wrap_limit_switch_callback(uint16_t GPIO_Pin);
#endif

void init_cpp_callback_wrap(void** motors);
void motor_pwm_callback(TIM_HandleTypeDef *htim);
void motor_timer_callback(TIM_HandleTypeDef *htim);
void heater_pwm_callback(TIM_HandleTypeDef *htim);
void heater_timer_callback(TIM_HandleTypeDef *htim);
void limit_switch_callback(uint16_t GPIO_Pin);


#endif /* INC_CPP_CALLBACK_WRAP_H_ */
