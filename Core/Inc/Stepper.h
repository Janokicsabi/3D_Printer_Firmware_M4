/*
 * Stepper.h
 *
 *  Created on: Nov 14, 2023
 *      Author: janok
 */

#ifndef SRC_HARDWARE_HANDLERS_STEPPER_H_
#define SRC_HARDWARE_HANDLERS_STEPPER_H_

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "timers.h"

#define SECONDS_IN_A_MINUTE			60
#define FULL_ROTATION_DEGREE		360.0f


class Stepper {
private:
	GPIO_TypeDef* step_port;
	uint16_t step_pin;
	GPIO_TypeDef* dir_port;
	uint16_t dir_pin;
	int32_t remaining_steps;
	uint8_t move_dir;
	float full_step_degree;
	uint32_t microstep_devider;

	const uint32_t calculate_motor_timer_period_from_speed(uint32_t move_speed, float one_step_displacement);
	void set_motor_timer_period(uint32_t timer_period);

	void start_motor_timer();

public:

	Stepper(GPIO_TypeDef* step_port, uint16_t step_pin, GPIO_TypeDef* dir_port, uint16_t dir_pin,
			float full_step_degree, uint32_t microstep_devider);
	Stepper();
	virtual ~Stepper();
	static void enable_steppers();
	static void disable_steppers();

	void set_goal_speed(float move_speed, float one_step_displacement);

	void motor_move_const(uint32_t step, uint8_t dir);
	void set_motor_speed(float move_speed, float one_step_displacement);
	void motor_move_accel(uint32_t step, uint8_t dir, uint32_t time_diff);
	void change_stepper_dir_pin(uint8_t new_dir);

	const uint32_t get_microstep_devider();
	TIM_HandleTypeDef* get_timer();
	const bool get_is_soft_pwm();
	const float get_full_step_degree();
	const bool is_stepper_moving();
	GPIO_TypeDef* get_step_port();
	uint16_t get_step_pin();

	void stop_motor_timer();
	void motor_timer_callback();
	void motor_PWM_callback();
};

#endif /* SRC_HARDWARE_HANDLERS_STEPPER_H_ */
