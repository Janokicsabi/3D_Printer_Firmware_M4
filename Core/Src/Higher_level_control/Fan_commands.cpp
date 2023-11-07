/*
 * Fan_commands.cpp
 *
 *  Created on: Oct 31, 2023
 *      Author: janok
 */

#include "Fan_commands.h"
#include "task_creator.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

extern EventGroupHandle_t command_state;
static Fan* hotend_fan;
static Fan* part_cooling_fan;


void fan_commands_init(Fan* hotend, Fan* part_cooling) {
	hotend_fan = hotend;
	part_cooling_fan = part_cooling;
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

//TODO Itt a duty cycle-t is kellene tudni állítani, ha belefér
void execute_M106(Command_struct* command) {
	//Set fan speed
	hotend_fan->turn_on_fan();
	part_cooling_fan->turn_on_fan();
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_M107(Command_struct* command) {
	//Turn off fan
	hotend_fan->turn_off_fan();
	part_cooling_fan->turn_off_fan();
}

