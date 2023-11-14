/*
 * motor_timer_callback.cpp
 *
 *  Created on: Oct 24, 2023
 *      Author: janok
 */


#include <cpp_callback_wrap.h>

#include <math.h>
#include <Axis.h>
#include <Motor.h>
#include "task_creator.h"
#include "Temp_controller.h"

static Axis* callback_axes[NUM_OF_AXES];
static Motor* callback_motors[NUM_OF_AXES];
static Limit_switch* callback_limit_switches[NUM_OF_AXES];
static uint16_t limit_switch_pins[NUM_OF_AXES];
static TIM_HandleTypeDef* callback_timers[NUM_OF_AXES];
static bool is_software_pwm[NUM_OF_AXES];
float displacement_per_microstep[NUM_OF_AXES];
GPIO_TypeDef* step_ports[NUM_OF_AXES];
uint16_t step_pins[NUM_OF_AXES];

static Temp_controller* hotend_heater;
static Temp_controller* bed_heater;
static TIM_HandleTypeDef* hotend_timer;
static TIM_HandleTypeDef* bed_timer;


void init_cpp_callback_wrap(void** axes, void* hotend, void* bed) {
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		callback_axes[i] = (Axis*)axes[i];
		callback_motors[i] = callback_axes[i]->get_motor();
		callback_limit_switches[i] = callback_axes[i]->get_limit_switch();
		limit_switch_pins[i] = callback_limit_switches[i]->get_switch_pin();
		callback_timers[i] = callback_motors[i]->get_timer();
		is_software_pwm[i] = callback_motors[i]->get_is_soft_pwm();
		displacement_per_microstep[i] = callback_axes[i]->get_one_step_displacement();
		step_ports[i] = callback_motors[i]->get_step_port();
		step_pins[i] = callback_motors[i]->get_step_pin();
	}

	hotend_heater = (Temp_controller*)hotend;
	bed_heater = (Temp_controller*)bed;
    hotend_timer = hotend_heater->get_timer();
    bed_timer = bed_heater->get_timer();
}

void cpp_wrap_motor_pwm_callback(TIM_HandleTypeDef *htim) {
	motor_pwm_callback(htim);
}

void motor_pwm_callback(TIM_HandleTypeDef *htim) {
    for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
    	if (is_software_pwm[i] == true && callback_timers[i] == htim) {
    		callback_motors[i]->motor_PWM_callback();
    		break;
        }
    }
}

void cpp_wrap_motor_timer_callback(TIM_HandleTypeDef *htim) {
	motor_timer_callback(htim);
}

void motor_timer_callback(TIM_HandleTypeDef *htim) {
    for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
    	if (callback_timers[i] == htim) {
    		callback_motors[i]->motor_timer_callback();
    		break;
        }
    }
}

void cpp_wrap_heater_pwm_callback(TIM_HandleTypeDef *htim) {
	heater_pwm_callback(htim);
}

void heater_pwm_callback(TIM_HandleTypeDef *htim) {
	if (htim == hotend_timer) {
		hotend_heater->heater_pin_off();
	}
	if (htim == bed_timer) {
		bed_heater->heater_pin_off();
	}
}

void cpp_wrap_heater_timer_callback(TIM_HandleTypeDef *htim) {
	heater_timer_callback(htim);
}

void heater_timer_callback(TIM_HandleTypeDef *htim) {
	if (htim == hotend_timer) {
		hotend_heater->heater_pin_on();
	}
	if (htim == bed_timer) {
		bed_heater->heater_pin_on();
	}
}

void cpp_wrap_limit_switch_callback(uint16_t GPIO_Pin) {
	limit_switch_callback(GPIO_Pin);
}

void limit_switch_callback(uint16_t GPIO_Pin) {
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (GPIO_Pin == limit_switch_pins[i]) {
			callback_limit_switches[i]->switch_pressed_callback();
			return;
		}
	}
}

const float time_delay = 0.0001;
uint32_t tick_num;														//Legyen nullázva mindig az elején
uint32_t required_steps[NUM_OF_AXES];
uint32_t steps_done[NUM_OF_AXES];													//Legyen nullázva mindig az elején
float s[NUM_OF_AXES];																//Legyen nullázva mindig az elején
uint8_t is_step_pin_active[NUM_OF_AXES];												//Legyen false mindig az elején
float acc[NUM_OF_AXES];
uint8_t are_steps_done;
float accel_time;
float v_max_reached[NUM_OF_AXES];
float acc_distance[NUM_OF_AXES];
float ellapsed_time;

void reset_motor_linear_acc_params(uint32_t* step_num, float* accel, float acc_time) {
	tick_num = 0;
	are_steps_done = 0;	//false
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		steps_done[i] = 0;
		required_steps[i] = step_num[i];
		s[i] = 0;
		is_step_pin_active[i] = false;
		acc[i] = accel[i];
		v_max_reached[i] = acc[i] * accel_time;
		acc_distance[i] = (required_steps[i] * displacement_per_microstep[i]) / 2.0;
		HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
	}
	accel_time = acc_time;
	ellapsed_time = 0;
}

uint64_t asd;

void __attribute__((optimize("O3")))motor_linear_acc(TIM_HandleTypeDef *htim) {
	uint32_t end_time;
	tick_num++;
	uint32_t start_time = TIM16->CNT;

	ellapsed_time += time_delay;
	float ellapsed_time_squared = ellapsed_time * ellapsed_time;
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (required_steps[i] != 0) {
			start_time = TIM16->CNT;
			if (steps_done[i] < required_steps[i] / 2.0) {
				s[i] = (acc[i] / 2.0) * ellapsed_time_squared;
			} else {
				s[i] = acc_distance[i] + v_max_reached[i] * ellapsed_time - (acc[i] / 2.0) * ellapsed_time_squared;
			}
			end_time = TIM16->CNT;

			if (is_step_pin_active[i]) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
				is_step_pin_active[i] = 0;
			}

			if (uint32_t(s[i] / displacement_per_microstep[i]) - steps_done[i] != 0) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_SET);
				steps_done[i]++;
				is_step_pin_active[i] = 1;

				if (steps_done[i] == required_steps[i] / 2.0) {
					ellapsed_time = 0;
				}
			}
			if (steps_done[i] == required_steps[i]) {
				required_steps[i] = 0;
			}
		}
	}

	asd += (end_time - start_time);

	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (required_steps[i] != 0) {
			return;
		}
	}
	HAL_TIM_Base_Stop_IT(htim);
}

