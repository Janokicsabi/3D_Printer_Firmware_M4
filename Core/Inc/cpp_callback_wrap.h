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
extern "C" void init_motor_timer_callback();
extern "C" void cpp_wrap_motor_pwm_callback(TIM_HandleTypeDef *htim);
extern "C" void cpp_wrap_motor_timer_callback(TIM_HandleTypeDef *htim);

extern "C" void cpp_wrap_heater_pwm_callback(TIM_HandleTypeDef *htim);
extern "C" void cpp_wrap_heater_timer_callback(TIM_HandleTypeDef *htim);


#endif


#endif /* INC_CPP_CALLBACK_WRAP_H_ */
