/*
 * motor_timer_callback.cpp
 *
 *  Created on: Oct 24, 2023
 *      Author: janok
 */


#include <cpp_callback_wrap.h>

#include <math.h>
#include <Axis.h>
#include <Descartes_Axis.h>
#include <Stepper.h>
#include "task_creator.h"
#include "Temp_controller.h"
#include "Fan.h"

static Axis* callback_axes[NUM_OF_AXES];
static Stepper* callback_motors[NUM_OF_AXES];
static Limit_switch* callback_limit_switches[NUM_OF_AXES];
static uint16_t limit_switch_pins[NUM_OF_AXES];
static TIM_HandleTypeDef* callback_timers[NUM_OF_AXES];
static bool is_software_pwm[NUM_OF_AXES];
float displacement_per_microstep[NUM_OF_AXES];
GPIO_TypeDef* step_ports[NUM_OF_AXES];
uint16_t step_pins[NUM_OF_AXES];

uint8_t is_moving[NUM_OF_AXES];
SemaphoreHandle_t sem_is_moving;

static Temp_controller* hotend_heater;
static Temp_controller* bed_heater;
static TIM_HandleTypeDef* hotend_timer;
static TIM_HandleTypeDef* bed_timer;

static Fan* partcooling_fan;
static TIM_HandleTypeDef* partcooling_timer;

extern EventGroupHandle_t command_state;


void init_cpp_callback_wrap(void** axes, void* hotend, void* bed, void* partcooling) {
	sem_is_moving = xSemaphoreCreateBinary();
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		callback_axes[i] = (Axis*)axes[i];
		callback_motors[i] = callback_axes[i]->get_motor();
		callback_limit_switches[i] = callback_axes[i]->get_limit_switch();
		limit_switch_pins[i] = callback_limit_switches[i]->get_switch_pin();
		displacement_per_microstep[i] = callback_axes[i]->get_one_step_displacement();
		step_ports[i] = callback_motors[i]->get_step_port();
		step_pins[i] = callback_motors[i]->get_step_pin();
	}

	hotend_heater = (Temp_controller*)hotend;
	bed_heater = (Temp_controller*)bed;
    hotend_timer = hotend_heater->get_timer();
    bed_timer = bed_heater->get_timer();
    partcooling_fan = (Fan*)partcooling;
    partcooling_timer = partcooling_fan->get_timer();
}

/*void cpp_wrap_motor_pwm_callback(TIM_HandleTypeDef *htim) {
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
}*/

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
	for (uint32_t i = 0; i < NUM_OF_DESCARTES_AXES; i++) {
		if (GPIO_Pin == limit_switch_pins[i]) {
			BaseType_t pxHigherPriorityTaskWoken;
			xSemaphoreTakeFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);
			is_moving[i] = 0;
			xSemaphoreGiveFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);
			return;
		}
	}
}

void cpp_wrap_fan_timer_callback(TIM_HandleTypeDef *htim) {
	if (partcooling_timer == htim) {
		fan_timer_callback();
	}
}

void fan_timer_callback() {
	partcooling_fan->fan_timer_callback();
}

void cpp_wrap_fan_pwm_callback(TIM_HandleTypeDef *htim) {
	if (partcooling_timer == htim) {
		fan_pwm_callback();
	}
}

void fan_pwm_callback() {
	partcooling_fan->fan_pwm_callback();
}

typedef enum {
	LINEAR,
	HOME
}Motor_move_type;

const float time_delay = 0.00005;
uint32_t tick_num;														//Legyen nullázva mindig az elején
uint32_t required_steps[NUM_OF_AXES];
uint32_t steps_done[NUM_OF_AXES];										//Legyen nullázva mindig az elején
float s[NUM_OF_AXES];													//Legyen nullázva mindig az elején
uint8_t is_step_pin_active[NUM_OF_AXES];								//Legyen false mindig az elején
float acc[NUM_OF_AXES];
uint8_t are_steps_done;
float accel_time[NUM_OF_AXES];
float v_max_reached[NUM_OF_AXES];
float acc_distance[NUM_OF_AXES];
Motor_move_type motor_move_type;
float total_ellapsed_time;
uint32_t section_counter;
float v[NUM_OF_AXES];
uint8_t move_phase[NUM_OF_AXES];


void callback_motor(TIM_HandleTypeDef* htim) {
	if (motor_move_type == LINEAR) {
		callback_motor_linear_acc(htim);
		//callback_motor_trapezoid_acc(htim);
	} else if (motor_move_type == HOME) {
		callback_motors_home(htim);
	}
}

void reset_motor_linear_acc_params(uint32_t* step_num, float* accel, uint8_t* move_dir) {
	motor_move_type = LINEAR;
	tick_num = 0;
	are_steps_done = 0;	//false
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		xSemaphoreTake(sem_is_moving, 1);
		if (step_num[i] > 0) {
			if (i >= NUM_OF_DESCARTES_AXES) {
				is_moving[i] = 1;
			} else if (((Descartes_Axis*)callback_axes[i])->can_motor_move(move_dir[i])) {
				is_moving[i] = 1;
			} else {
				is_moving[i] = 0;
			}
		}
		else {
			is_moving[i] = 0;
		}
		xSemaphoreGive(sem_is_moving);

		accel_time[i] = 0;
		steps_done[i] = 0;
		required_steps[i] = step_num[i];
		s[i] = 0;
		is_step_pin_active[i] = false;
		acc[i] = accel[i];
		acc_distance[i] = (required_steps[i] * displacement_per_microstep[i]) / 2.0;
		total_ellapsed_time = 0.0f;
		section_counter = 0;
		v[i] = 0.0f;
		move_phase[i] = 0;
		HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
	}
}

uint64_t asd = 0;
uint32_t worst_time = 0;

void __attribute__((optimize("O3"))) callback_motor_linear_acc(TIM_HandleTypeDef *htim) {
	uint32_t end_time;
	tick_num++;
	uint32_t start_time = TIM16->CNT;

	total_ellapsed_time += time_delay;

	uint8_t is_moving_local[NUM_OF_AXES];
	BaseType_t pxHigherPriorityTaskWoken;
	xSemaphoreTakeFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		is_moving_local[i] = is_moving[i];
	}
	xSemaphoreGiveFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);

	/*for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (required_steps[i] > 0 && is_moving_local[i] != 0) {
			if (total_ellapsed_time < accel_time) {
				s[i] = (acc[i] / 2.0f) * ellapsed_time_squared;
			} else {
				s[i] = acc_distance[i] + v_max_reached[i] * ellapsed_time - (acc[i] / 2.0f) * ellapsed_time_squared;
			}

			if (is_step_pin_active[i]) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
				is_step_pin_active[i] = 0;
			}

			if (uint32_t(s[i] / displacement_per_microstep[i]) - steps_done[i] != 0) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_SET);
				steps_done[i]++;
				is_step_pin_active[i] = 1;

				if (total_ellapsed_time >= accel_time && section_counter == 0) {
					section_counter++;
					ellapsed_time = 0;
				}
			}
		}
	}

	if (total_ellapsed_time < 2 * accel_time) {
		end_time = TIM16->CNT;
		asd += (end_time - start_time);
		if (worst_time < end_time - start_time) {
			worst_time = end_time - start_time;
		}
		return;
	}*/

 	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (required_steps[i] > 0 && is_moving_local[i] != 0) {
			if (move_phase[i] == 0) {
				s[i] = (acc[i] / 2.0f) * total_ellapsed_time * total_ellapsed_time;
			} else {
				float time_since_new_phase = total_ellapsed_time - accel_time[i];
				s[i] = acc_distance[i] + v_max_reached[i] * time_since_new_phase - (acc[i] / 2.0f) * time_since_new_phase * time_since_new_phase;
			}

			if (is_step_pin_active[i]) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
				is_step_pin_active[i] = 0;
			}

			else if (uint32_t(s[i] / displacement_per_microstep[i]) - steps_done[i] != 0) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_SET);
				steps_done[i]++;
				is_step_pin_active[i] = 1;

				if (steps_done[i] == required_steps[i] >> 1) {
					move_phase[i]++;
					acc_distance[i] = s[i];
					accel_time[i] = total_ellapsed_time;
					v_max_reached[i] = acc[i] * total_ellapsed_time;
				}
				if (steps_done[i] == required_steps[i]) {
					required_steps[i] = 0;
				}
			}
		}
	}

	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (required_steps[i] != 0) {
			end_time = TIM16->CNT;
			asd += (end_time - start_time);
			if (worst_time < end_time - start_time) {
				worst_time = end_time - start_time;
			}
			return;
		}
	}

	end_time = TIM16->CNT;
	asd += (end_time - start_time);
	if (worst_time < end_time - start_time) {
		worst_time = end_time - start_time;
	}
	HAL_TIM_Base_Stop_IT(htim);

	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(command_state, READY_FOR_NEXT_COMMAND, &xHigherPriorityTaskWoken );
	if( xResult != pdFAIL ) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
float full_distance[NUM_OF_AXES];
uint32_t acc_steps_end[NUM_OF_AXES];
uint32_t const_steps_end[NUM_OF_AXES];
float start_speeds[NUM_OF_AXES];
float section_ellapsed_time;
float const_speed_time;

void reset_motor_trapezoid_params(uint32_t* step_num, float* accel, float acc_time, float const_time, float* full_dist, uint8_t* move_dir, float* start_speed) {
	motor_move_type = LINEAR;
	tick_num = 0;
	are_steps_done = 0;	//false
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		xSemaphoreTake(sem_is_moving, 1);
		if (step_num[i] > 0) {
			if (i > NUM_OF_DESCARTES_AXES) {
				is_moving[i] = 1;
			} else if (((Descartes_Axis*)callback_axes[i])->can_motor_move(move_dir[i])) {
				is_moving[i] = 1;
			} else {
				is_moving[i] = 0;
			}
		}
		else {
			is_moving[i] = 0;
		}
		xSemaphoreGive(sem_is_moving);

		full_distance[i] = full_dist[i];
		const_speed_time = const_time;

		steps_done[i] = 0;
		required_steps[i] = step_num[i];
		s[i] = 0;
		is_step_pin_active[i] = false;
		acc[i] = accel[i];
		start_speeds[i] = start_speed[i];
		total_ellapsed_time = 0.0f;
		section_counter = 0;
		HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
	}
}

/*void callback_motor_trapezoid_acc(TIM_HandleTypeDef *htim) {
	uint32_t end_time;
	tick_num++;
	uint32_t start_time = TIM16->CNT;

	total_ellapsed_time += time_delay;
	section_ellapsed_time += time_delay;
	//float section_ellapsed_time_squared = ellapsed_time * ellapsed_time;

	uint8_t is_moving_local[NUM_OF_AXES];
	BaseType_t pxHigherPriorityTaskWoken;
	xSemaphoreTakeFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		is_moving_local[i] = is_moving[i];
	}
	xSemaphoreGiveFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);

	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (required_steps[i] > 0 && is_moving_local[i] != 0) {
			if (total_ellapsed_time < accel_time) {
				s[i] = (acc[i] / 2.0f) * section_ellapsed_time_squared;
			} else if (total_ellapsed_time < accel_time + const_speed_time) {
				s[i] = acc_distance[i] + v_max_reached[i] * section_ellapsed_time;
			} else {
				s[i] = full_distance[i] - acc_distance[i] + v_max_reached[i] * section_ellapsed_time - (acc[i] / 2.0f) * section_ellapsed_time_squared;
			}

			if (is_step_pin_active[i]) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
				is_step_pin_active[i] = 0;
			}

			if (uint32_t(s[i] / displacement_per_microstep[i]) - steps_done[i] != 0) {
				HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_SET);
				steps_done[i]++;
				is_step_pin_active[i] = 1;

				if (total_ellapsed_time >= accel_time && section_counter == 0) {
					section_counter++;
					section_ellapsed_time = 0;
				} else if (total_ellapsed_time >= accel_time + const_speed_time && section_counter == 1) {
					section_counter++;
					section_ellapsed_time = 0;
				}
			}
		}
	}

	if (total_ellapsed_time < 2 * accel_time + const_speed_time) {
		end_time = TIM16->CNT;
		asd += (end_time - start_time);
		if (worst_time < end_time - start_time) {
			worst_time = end_time - start_time;
		}
		return;
	}

	end_time = TIM16->CNT;
	asd += (end_time - start_time);
	if (worst_time < end_time - start_time) {
		worst_time = end_time - start_time;
	}
	HAL_TIM_Base_Stop_IT(htim);

	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(command_state, READY_FOR_NEXT_COMMAND, &xHigherPriorityTaskWoken );
	if( xResult != pdFAIL ) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////

float home_acc;
float home_max_speed;
float acc_time[NUM_OF_DESCARTES_AXES];

void reset_motors_home_params(uint8_t* is_going_home) {
	motor_move_type = HOME;
	home_acc = 1;				//mm/s^2
	home_max_speed = 2;			//mm/s
	total_ellapsed_time = 0;
	tick_num = 0;
	for (uint32_t i = 0; i < NUM_OF_DESCARTES_AXES; i++) {
		v[i] = 0.0f;
		steps_done[i] = 0;
		xSemaphoreTake(sem_is_moving, 1);
		if (is_going_home[i] != 0 && ((Descartes_Axis*)callback_axes[i])->can_motor_move(((Descartes_Axis*)callback_axes[i])->get_limit_switch_dir())) {
			is_moving[i] = 1;
		} else {
			is_moving[i] = 0;
		}
		xSemaphoreGive(sem_is_moving);
	}
}

void callback_motors_home(TIM_HandleTypeDef *htim) {
	uint32_t end_time;
	tick_num++;
	uint32_t start_time = TIM16->CNT;

	total_ellapsed_time += time_delay;
	float ellapsed_time_squared = total_ellapsed_time * total_ellapsed_time;

	uint8_t is_moving_local[NUM_OF_DESCARTES_AXES];
	BaseType_t pxHigherPriorityTaskWoken;
	xSemaphoreTakeFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);
	for (uint32_t i = 0; i < NUM_OF_DESCARTES_AXES; i++) {
		is_moving_local[i] = is_moving[i];
	}
	xSemaphoreGiveFromISR(sem_is_moving, &pxHigherPriorityTaskWoken);

	for (uint32_t i = 0; i < NUM_OF_DESCARTES_AXES; i++) {
		if (is_moving_local[i] == 0) {
			continue;
		}

		if (v[i] <= home_max_speed) {
			v[i] = home_acc * total_ellapsed_time;
			s[i] = (home_acc / 2.0f) * ellapsed_time_squared;
			acc_time[i] = total_ellapsed_time;
		} else {
			s[i] = (home_acc / 2.0f) * acc_time[i] * acc_time[i] + home_max_speed * (total_ellapsed_time - acc_time[i]);
		}

		if (is_step_pin_active[i]) {
			HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_RESET);
			is_step_pin_active[i] = 0;
		}

		if (uint32_t(s[i] / displacement_per_microstep[i]) - steps_done[i] != 0) {
			HAL_GPIO_WritePin(step_ports[i], step_pins[i], GPIO_PIN_SET);
			steps_done[i]++;
			is_step_pin_active[i] = 1;
		}
	}

	for (uint32_t i = 0; i < NUM_OF_DESCARTES_AXES; i++) {
		if (is_moving_local[i] != 0) {
			end_time = TIM16->CNT;
			asd += (end_time - start_time);
			if (worst_time < end_time - start_time) {
				worst_time = end_time - start_time;
			}
			return;
		}
	}

	end_time = TIM16->CNT;
	asd += (end_time - start_time);
	if (worst_time < end_time - start_time) {
		worst_time = end_time - start_time;
	}
	HAL_TIM_Base_Stop_IT(htim);

	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(command_state, READY_FOR_NEXT_COMMAND, &xHigherPriorityTaskWoken );
	if( xResult != pdFAIL ) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
