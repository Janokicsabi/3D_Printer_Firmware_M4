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

extern "C" void reset_motor_linear_acc_params(uint32_t* step_num, float* accel, float acc_time);
extern "C" void motor_linear_acc(TIM_HandleTypeDef *htim);
#endif

void init_cpp_callback_wrap(void** axes, void* hotend_heater, void* bed_heater);
void motor_pwm_callback(TIM_HandleTypeDef *htim);
void motor_timer_callback(TIM_HandleTypeDef *htim);
void heater_pwm_callback(TIM_HandleTypeDef *htim);
void heater_timer_callback(TIM_HandleTypeDef *htim);
void limit_switch_callback(uint16_t GPIO_Pin);


#endif /* INC_CPP_CALLBACK_WRAP_H_ */
