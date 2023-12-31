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

extern "C" void reset_motor_linear_acc_params(uint32_t* step_num, float* accel, uint8_t* dirs);
extern "C" void callback_motor_linear_acc(TIM_HandleTypeDef *htim);

extern "C"  void reset_motor_trapezoid_params(uint32_t* step_num, float* accel, float acc_time, float const_time, float* full_dist, uint8_t* move_dir, float* start_speed);
extern "C"  void callback_motor_trapezoid_acc(TIM_HandleTypeDef *htim);

extern "C" void reset_motors_home_params(uint8_t* is_going_home);
extern "C" void callback_motors_home(TIM_HandleTypeDef *htim);
extern "C" void callback_motor(TIM_HandleTypeDef* htim);

extern "C" void cpp_wrap_fan_timer_callback(TIM_HandleTypeDef *htim);
extern "C" void cpp_wrap_fan_pwm_callback(TIM_HandleTypeDef *htim);
#endif

void init_cpp_callback_wrap(void** axes, void* hotend_heater, void* bed_heater, void* partcooling);
void motor_pwm_callback(TIM_HandleTypeDef *htim);
void motor_timer_callback(TIM_HandleTypeDef *htim);
void heater_pwm_callback(TIM_HandleTypeDef *htim);
void heater_timer_callback(TIM_HandleTypeDef *htim);
void limit_switch_callback(uint16_t GPIO_Pin);
void fan_timer_callback();
void fan_pwm_callback();


#endif /* INC_CPP_CALLBACK_WRAP_H_ */
