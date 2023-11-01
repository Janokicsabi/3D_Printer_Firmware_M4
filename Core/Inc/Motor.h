/*
 * Motor.h
 *
 *  Created on: Oct 15, 2023
 *      Author: janok
 */

#ifndef SRC_MOTOR_H_
#define SRC_MOTOR_H_

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

#define SECONDS_IN_A_MINUTE			60
#define FULL_ROTATION_DEGREE		360.0f
#define MOTOR_BASIC_STEP_DEGREE		1.8f

#define MOTOR_MOVING				(1 << 0)
#define MOTOR_STANDING				(1 << 1)


class Motor {
private:
	GPIO_TypeDef* step_port;
	uint16_t step_pin;
	GPIO_TypeDef* dir_port;
	uint16_t dir_pin;
	TIM_HandleTypeDef* timer;
	uint32_t timer_channel;
	uint32_t remaining_steps;
	uint8_t move_dir;
	uint32_t microstep_devider;
	uint32_t feedrate_accel_time_diff;
	float motor_speed;
	bool is_soft_pwm;
	EventGroupHandle_t motor_state;

	const uint32_t calculate_motor_timer_period_from_speed(uint32_t move_speed, float one_step_displacement);
	void set_motor_timer_period(uint32_t timer_period);
	void change_motor_dir_pin(uint8_t new_dir);

	void start_motor_timer();

public:

	Motor(GPIO_TypeDef* step_port, uint16_t step_pin, GPIO_TypeDef* dir_port, uint16_t dir_pin,
			TIM_HandleTypeDef* timer, uint32_t timer_channel, uint32_t microstep_devider, bool is_soft_pwm);
	Motor();
	virtual ~Motor();
	static void enable_motors();
	static void disable_motors();

	void motor_move_const(uint32_t step, uint8_t dir);
	void set_motor_speed(float move_speed, float one_step_displacement);
	void motor_move_accel(uint32_t step, uint8_t dir, uint32_t time_diff);

	const uint32_t get_microstep_devider();
	TIM_HandleTypeDef* get_timer();
	const bool get_is_soft_pwm();
	const bool is_motor_moving();

	void stop_motor_timer();
	void motor_timer_callback();
	void motor_PWM_callback();
};

#endif /* SRC_MOTOR_H_ */
