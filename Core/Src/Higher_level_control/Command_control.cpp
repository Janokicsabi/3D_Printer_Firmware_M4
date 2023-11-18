/*
 * Command_control.cpp
 *
 *  Created on: Oct 30, 2023
 *      Author: janok
 */

#include "Command_control.h"
#include "Axis_commands.h"
#include "Temp_commands.h"
#include "Fan_commands.h"
#include <cstring>

extern EventGroupHandle_t command_state;
extern xQueueHandle queue_command;


Command_executor supported_commands[] = {
		//Axis commands
		{"G1", execute_G1},
		{"G21", execute_G21},
		{"G28", execute_G28},
		{"G90", execute_G90},
		{"G92", execute_G92},
		{"M82", execute_M82},
		{"M84", execute_M84},
		//Temp commands
		{"M104", execute_M104},
		{"M109", execute_M109},
		{"M140", execute_M140},
		{"M190", execute_M190},
		//Fan commands
		{"M106", execute_M106},
		{"M107", execute_M107}
};


void init_command_control() {
}

void task_command_control(void* param) {
	//TODO Remélhetőleg nem dominálja le teljesen a többi taskot, erre figyelni!
	while(1) {
		EventBits_t command_status = xEventGroupWaitBits(command_state, READY_FOR_NEXT_COMMAND, pdFALSE, pdFALSE, portMAX_DELAY);
		if((command_status & READY_FOR_NEXT_COMMAND) != 0) {
			Command_struct received_command;
			int32_t command_index = get_last_command_from_queue(&received_command);
			if (command_index != -1) {
				xEventGroupClearBits(command_state, READY_FOR_NEXT_COMMAND);
   				supported_commands[command_index].Command_executor(&received_command);
			} else {
				vTaskDelay(10);
			}
		}
	}
}

int32_t get_command_index(char* code) {
    int32_t num_supported_commands = (int32_t)((float)sizeof(supported_commands) / sizeof(supported_commands[0]));
    int32_t command_index = -1;
    for (int32_t i = 0; i < num_supported_commands; i++) {
        if (strcmp(code, supported_commands[i].command) == 0) {
            command_index = i;
            break;
        }
    }
    return command_index;
}

int32_t get_last_command_from_queue(Command_struct* last_command) {
	int32_t command_index = -1;
	if (xQueueReceive(queue_command, (void*)last_command, 0) == pdPASS) {
		command_index = get_command_index(last_command->command_code);
		if (command_index == -1) {
			//ERROR: Unsupported command
		}
	} else {
		 //Couldn't receive command from queue
	}
	return command_index;
}
