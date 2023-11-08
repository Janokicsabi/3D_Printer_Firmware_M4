/*
 * motor_timer_callback.cpp
 *
 *  Created on: Oct 24, 2023
 *      Author: janok
 */

#include <Axis.h>
#include <cpp_callback_wrap.h>
#include <Motor.h>
#include "task_creator.h"
#include "Temp_controller.h"

static Axis* callback_axes[NUM_OF_AXES];
static Motor* callback_motors[NUM_OF_AXES];
static Limit_switch* callback_limit_switches[NUM_OF_AXES];
static uint16_t limit_switch_pins[NUM_OF_AXES];
static TIM_HandleTypeDef* callback_timers[NUM_OF_AXES];
static bool is_software_pwm[NUM_OF_AXES];

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

