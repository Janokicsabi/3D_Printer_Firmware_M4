/*
 * Limitswitch.h
 *
 *  Created on: Oct 26, 2023
 *      Author: janok
 */

#ifndef SRC_HARDWARE_HANDLERS_LIMITSWITCH_H_
#define SRC_HARDWARE_HANDLERS_LIMITSWITCH_H_

#include "main.h"

#include <Motor.h>


class Limit_switch {
private:
	GPIO_TypeDef* switch_port;
	uint32_t switch_pin;
	Motor* motor;

public:
	Limit_switch(GPIO_TypeDef* switch_port, uint32_t switch_pin, Motor* motor);
	virtual ~Limit_switch();

	bool is_switch_pressed();
	void switch_pressed_callback();
};

#endif /* SRC_HARDWARE_HANDLERS_LIMITSWITCH_H_ */
