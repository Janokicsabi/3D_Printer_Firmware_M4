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
	void (*Command_executor)(Command_struct*);
}Command_executor;

void init_command_control();
void task_command_control(void* params);
int32_t get_last_command_from_queue(Command_struct* last_command);
void task_print_finished_final_steps(void* param);
void filament_test(void* param);



#endif /* SRC_HARDWARE_HANDLERS_COMMAND_CONTROL_H_ */
