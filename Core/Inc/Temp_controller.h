/*
 * Temp_controller.h
 *
 *  Created on: Oct 31, 2023
 *      Author: janok
 */

#ifndef SRC_HIGHER_LEVEL_CONTROL_TEMP_CONTROLLER_H_
#define SRC_HIGHER_LEVEL_CONTROL_TEMP_CONTROLLER_H_

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

extern SemaphoreHandle_t duty_cycle_sem;
extern SemaphoreHandle_t goal_temp_sem;

#define MIN_GOAL_TEMP		25
#define MAX_GOAL_TEMP		250
#define TEMP_ARRAY_SIZE		50


class Temp_controller {
private:
	GPIO_TypeDef* heater_port;
	uint16_t heater_pin;
	TIM_HandleTypeDef* timer;
	uint32_t timer_channel;
	float duty_cycle;
	float goal_temp;
	float current_temp_celsius;
	float Kp;
	float Ki;
	float Ti_PI;
	float integral_sum;
	bool is_soft_pwm;

	float goal_error_tolerance;
	float last_measured_temps[TEMP_ARRAY_SIZE];
	SemaphoreHandle_t sem_temp_array;
	uint32_t temp_index;

	void modify_timer_compare();

public:
	Temp_controller(TIM_HandleTypeDef* timer, uint32_t timer_channel, float Kp, float Ki, float Ti_PI, float goal_error_tolerance, bool is_soft_pwm);
	virtual ~Temp_controller();

	float compute_PI_controller();
	TIM_HandleTypeDef* get_timer();
	void set_duty_cycle(float new_duty_cycle);
	float get_duty_cycle();
	void set_goal_temp(float new_goal_temp);
	float get_goal_temp();
	void set_current_temp_celius(float current_temp_celsius);
	float get_current_temp_celius();
	bool is_goal_temp_reached();
	void reset_controller_variables();
	void heater_timer_start();
	void heater_timer_stop();
	void heater_pin_on();
	void heater_pin_off();
};

void task_hotend_control(void* param);
void task_temp_log(void* param);

#endif /* SRC_HIGHER_LEVEL_CONTROL_TEMP_CONTROLLER_H_ */
