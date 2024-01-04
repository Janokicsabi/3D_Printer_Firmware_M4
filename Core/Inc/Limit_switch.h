/*
 * Limitswitch.h
 *
 *  Created on: Oct 26, 2023
 *      Author: janok
 */

#ifndef SRC_HARDWARE_HANDLERS_LIMITSWITCH_H_
#define SRC_HARDWARE_HANDLERS_LIMITSWITCH_H_

#include <Stepper.h>
#include "main.h"



class Limit_switch {
private:
	GPIO_TypeDef* switch_port;
	uint16_t switch_pin;
	Stepper* motor;

public:
	Limit_switch();
	Limit_switch(GPIO_TypeDef* switch_port, uint16_t switch_pin, Stepper* motor);
	virtual ~Limit_switch();

	bool is_switch_pressed();
	uint16_t get_switch_pin();
	void switch_pressed_callback();
};

#endif /* SRC_HARDWARE_HANDLERS_LIMITSWITCH_H_ */
