/*
 * Temp_commands.h
 *
 *  Created on: Oct 31, 2023
 *      Author: janok
 */

#ifndef INC_TEMP_COMMANDS_H_
#define INC_TEMP_COMMANDS_H_

#include "main.h"
#include <Command.h>
#include "Temp_controller.h"


void temp_commands_init(Temp_controller* hotend, Temp_controller* bed);
void execute_M104(Command_struct* command);
void execute_M109(Command_struct* command);
void execute_M140(Command_struct* command);
void execute_M190(Command_struct* command);

void start_temperature_control(Temp_controller* heater, Command_struct* command);
void stop_temperature_control(Temp_controller* heater);
void task_wait_for_temp_to_reach(void* param);

#endif /* INC_TEMP_COMMANDS_H_ */
