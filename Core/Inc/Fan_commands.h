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
#include "Fan.h"


void fan_commands_init(Fan* hotend, Fan* part_cooling);
void execute_M106(Command_struct* command);
void execute_M107(Command_struct* command);


#endif /* INC_FAN_COMMANDS_H_ */
