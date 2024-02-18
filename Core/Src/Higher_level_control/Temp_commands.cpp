/*
 * Temp_commands.cpp
 *
 *  Created on: Oct 31, 2023
 *      Author: janok
 */

#include "Temp_commands.h"
#include "thermistor.h"
#include "task_creator.h"
#include "Fan.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

extern EventGroupHandle_t command_state;
static Temp_controller* hotend_heater;
static Temp_controller* bed_heater;
static Fan* hotend_fan;


void temp_commands_init(Temp_controller* hotend, Temp_controller* bed, Fan* fan) {
	hotend_heater = hotend;
	bed_heater = bed;
	hotend_fan = fan;
}

void execute_M104(Command_struct* command) {
	//Start the hotend heating process
	//Doesn't wait for the goal temperature to be reached
	hotend_fan->turn_on_fan();
	xTaskCreate(task_temp_read, "TEMP_READ", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);
	xTaskCreate(task_hotend_control, "HOTEND_TEMP_CONTROL", TASK_MID_STACK_SIZE, (void*)hotend_heater, TASK_MID_PRIO, NULL);
	start_temperature_control(hotend_heater, command);
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_M109(Command_struct* command) {
	//Start the hotend heating process
	//Waits for the goal temperature to be reached
	hotend_fan->turn_on_fan();
	xTaskCreate(task_temp_read, "TEMP_READ", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);
	xTaskCreate(task_hotend_control, "HOTEND_TEMP_CONTROL", TASK_MID_STACK_SIZE, (void*)hotend_heater, TASK_MID_PRIO, NULL);
	start_temperature_control(hotend_heater, command);
	xTaskCreate(task_wait_for_temp_to_reach, "TEMP_REACH_HOTEND", TASK_SMALL_STACK_SIZE, (void*)hotend_heater, TASK_LOW_PRIO, NULL);
}

void execute_M140(Command_struct* command) {
	//Start the bed heating process
	//Doesn't wait for the goal temperature to be reached
	xTaskCreate(task_temp_read, "TEMP_READ", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);
	xTaskCreate(task_bed_control, "BED_TEMP_CONTROL", TASK_MID_STACK_SIZE, (void*)bed_heater, TASK_MID_PRIO, NULL);
	start_temperature_control(bed_heater, command);
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_M190(Command_struct* command) {
	//Start the bed heating process
	//Waits for the goal temperature to be reached
	xTaskCreate(task_temp_read, "TEMP_READ", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);
	xTaskCreate(task_bed_control, "BED_TEMP_CONTROL", TASK_MID_STACK_SIZE, (void*)bed_heater, TASK_MID_PRIO, NULL);
	start_temperature_control(bed_heater, command);
	xTaskCreate(task_wait_for_temp_to_reach, "TEMP_REACH_BED", TASK_SMALL_STACK_SIZE, (void*)bed_heater, TASK_LOW_PRIO, NULL);
}

void start_temperature_control(Temp_controller* heater, Command_struct* command) {
	if (command->s != INVALID_COMMAND_PARAM) {
		float goal_temp = command->s;
		heater->reset_controller_variables();
		heater->set_goal_temp(goal_temp);
		heater->heater_timer_start();
	} else {
		//ERROR, no goal temperature provided
		stop_temperature_control(heater);
	}
}

void stop_temperature_control(Temp_controller* heater) {
	heater->heater_timer_stop();
}

void task_wait_for_temp_to_reach(void* param) {
	Temp_controller* heater = (Temp_controller*)param;
	while(heater->is_goal_temp_reached() != true) {
		vTaskDelay(100);
	}
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
	vTaskDelete(NULL);
}
