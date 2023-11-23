/*
 * Fan.h
 *
 *  Created on: Oct 29, 2023
 *      Author: janok
 */

#ifndef SRC_HARDWARE_HANDLERS_FAN_H_
#define SRC_HARDWARE_HANDLERS_FAN_H_

#include "main.h"

//TODO Duty cycle implement
class Fan {
private:
	GPIO_TypeDef* fan_port;
	uint16_t fan_pin;

	TIM_HandleTypeDef* timer;
	uint32_t timer_channel;
	float duty_cycle;

public:
	Fan(GPIO_TypeDef* fan_port, uint16_t fan_pin);
	Fan(GPIO_TypeDef* fan_port, uint16_t fan_pin, TIM_HandleTypeDef* timer, uint32_t timer_channel);
	virtual ~Fan();

	void turn_on_fan();
	void turn_off_fan();

	void set_duty_cycle(float duty_cycle);
	void start_fan_timer();
	void stop_fan_timer();
	void fan_timer_callback();
	void fan_pwm_callback();
	TIM_HandleTypeDef* get_timer();
};

#endif /* SRC_HARDWARE_HANDLERS_FAN_H_ */
