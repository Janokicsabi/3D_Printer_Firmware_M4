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

void execute_M106(Command_struct* command) {
	//Set fan speed
	if (command->s != INVALID_COMMAND_PARAM) {
		part_cooling_fan->set_duty_cycle(command->s);
		part_cooling_fan->start_fan_timer();
	}
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_M107(Command_struct* command) {
	//Turn off fan
	part_cooling_fan->stop_fan_timer();
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

