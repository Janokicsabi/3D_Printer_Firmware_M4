/*
 * Fan_command.h
 *
 *  Created on: Oct 31, 2023
 *      Author: janok
 */

#ifndef INC_FAN_COMMANDS_H_
#define INC_FAN_COMMANDS_H_

#include "main.h"
#include <Command.h>


void fan_commands_init();
void execute_M106(Possible_params* command);
void execute_M107(Possible_params* command);


#endif /* INC_FAN_COMMANDS_H_ */
