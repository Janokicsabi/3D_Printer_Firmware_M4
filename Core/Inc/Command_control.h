/*
 * Command_control.h
 *
 *  Created on: Oct 30, 2023
 *      Author: janok
 */

#ifndef SRC_HARDWARE_HANDLERS_COMMAND_CONTROL_H_
#define SRC_HARDWARE_HANDLERS_COMMAND_CONTROL_H_

#include <Command.h>

typedef struct {
	char command[MAX_COMMAND_SIZE];
	void (*Command_executor)(Possible_params*);
}Command_executor;

void init_command_control();
void task_command_control(void* params);



#endif /* SRC_HARDWARE_HANDLERS_COMMAND_CONTROL_H_ */
