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


void temp_commands_init();
void execute_M104(Possible_params* command);
void execute_M109(Possible_params* command);
void execute_M140(Possible_params* command);
void execute_M190(Possible_params* command);

void start_temperature_control(Temp_controller* heater, Possible_params* command);
void stop_temperature_control(Temp_controller* heater);
void task_wait_for_temp_to_reach(void* param);

#endif /* INC_TEMP_COMMANDS_H_ */
