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

public:
	Fan(GPIO_TypeDef* fan_port, uint16_t fan_pin);
	virtual ~Fan();

	void turn_on_fan();
	void turn_off_fan();
};

#endif /* SRC_HARDWARE_HANDLERS_FAN_H_ */
