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

extern Axis* axes_array[NUM_OF_AXES];
static Motor* callback_motors[NUM_OF_AXES];
static TIM_HandleTypeDef* callback_timers[NUM_OF_AXES];
static bool is_software_pwm[NUM_OF_AXES];

TIM_HandleTypeDef* hotend_timer;
extern Temp_controller* hotend_heater;
TIM_HandleTypeDef* bed_timer;
extern Temp_controller* bed_heater;


void init_cpp_callback_wrap() {
    for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
    	callback_motors[i] = axes_array[i]->get_motor();
    	callback_timers[i] = callback_motors[i]->get_timer();
    	is_software_pwm[i] = callback_motors[i]->get_is_soft_pwm();
    }
    hotend_timer = hotend_heater->get_timer();
    bed_timer = bed_heater->get_timer();
}

void cpp_wrap_motor_pwm_callback(TIM_HandleTypeDef *htim) {
    for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
    	if (is_software_pwm[i] == true && callback_timers[i] == htim) {
    		callback_motors[i]->motor_PWM_callback();
        }
    }
}

void cpp_wrap_motor_timer_callback(TIM_HandleTypeDef *htim) {
    for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
    	if (callback_timers[i] == htim) {
    		callback_motors[i]->motor_timer_callback();
        }
    }
}

void cpp_wrap_heater_pwm_callback(TIM_HandleTypeDef *htim) {
	if (htim == hotend_timer) {
		hotend_heater->heater_pin_off();
	}
	if (htim == bed_timer) {
		bed_heater->heater_pin_off();
	}
}

void cpp_wrap_heater_timer_callback(TIM_HandleTypeDef *htim) {
	if (htim == hotend_timer) {
		hotend_heater->heater_pin_on();
	}
	if (htim == bed_timer) {
		bed_heater->heater_pin_on();
	}
}

