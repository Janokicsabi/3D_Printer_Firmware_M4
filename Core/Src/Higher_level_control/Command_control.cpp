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
extern EventGroupHandle_t event_command_read_ready;
extern xQueueHandle queue_command;

extern Temp_controller* hotend_heater;
extern Temp_controller* bed_heater;
extern Fan* fan_hotend;


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

uint32_t num_of_free_spaces = 0;

void task_command_control(void* param) {
	//TODO Remélhetőleg nem dominálja le teljesen a többi taskot, erre figyelni!
	while(1) {
		EventBits_t command_read_state =  xEventGroupGetBits(event_command_read_ready);
		if (command_read_state & COMMAND_READ_FINISHED != 0 && uxQueueSpacesAvailable(queue_command) == MESSAGE_QUEUE_SIZE) {
			xEventGroupSetBits(command_state, ALL_COMMANDS_EXECUTED);
			xTaskCreate(task_print_finished_final_steps, "FINAL_TASK", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);
			vTaskDelete(NULL);
		}

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

void task_print_finished_final_steps(void* param) {
	xEventGroupClearBits(command_state, READY_FOR_NEXT_COMMAND);
	Command_struct rise_head;
	rise_head.x = INVALID_COMMAND_PARAM;
	rise_head.y = INVALID_COMMAND_PARAM;
	rise_head.z = 10.0f;
	rise_head.e = INVALID_COMMAND_PARAM;
	rise_head.f = 1200.0f;
	execute_G1(&rise_head);
	EventBits_t command_status = xEventGroupWaitBits(command_state, READY_FOR_NEXT_COMMAND, pdTRUE, pdFALSE, portMAX_DELAY);

	//STOP HEATING
	hotend_heater->heater_timer_stop();
	bed_heater->heater_timer_stop();

	const float HOTEND_FAN_OFF_TEMP = 50.0f;
	while(1) {
		if (hotend_heater->get_current_temp_celius() < HOTEND_FAN_OFF_TEMP) {
			fan_hotend->turn_off_fan();
			vTaskDelete(NULL);
		}
		vTaskDelay(200);
	}
}

void filament_test(void* param) {
	Command* c = new Command();
	Command_struct* p_p;

	xEventGroupWaitBits(command_state, READY_FOR_NEXT_COMMAND, pdTRUE, pdFALSE, portMAX_DELAY);
	char go_home[50] = "G28\n";
	c->set_code_and_param_string(go_home);
	c->extract_params_from_command_string();
	p_p = c->get_params();
	execute_G28(p_p);

	xEventGroupWaitBits(command_state, READY_FOR_NEXT_COMMAND, pdTRUE, pdFALSE, portMAX_DELAY);

	char go_to_center[50] = "G1 X90.0 Y30.0 Z30.0 F1000\n";
	c->set_code_and_param_string(go_to_center);
	c->extract_params_from_command_string();
	p_p = c->get_params();
	execute_G1(p_p);

	xEventGroupWaitBits(command_state, READY_FOR_NEXT_COMMAND, pdTRUE, pdFALSE, portMAX_DELAY);

	char heat_up_hotend[50] = "M109 S220.0\n";
	c->set_code_and_param_string(heat_up_hotend);
	c->extract_params_from_command_string();
	p_p = c->get_params();
	execute_M109(p_p);

	xEventGroupWaitBits(command_state, READY_FOR_NEXT_COMMAND, pdTRUE, pdFALSE, portMAX_DELAY);

	hotend_heater->heater_timer_stop();
	task_print_finished_final_steps(NULL);

	char extrude_filament[50] = "G1 E10.0 F60.0\n";
	c->set_code_and_param_string(extrude_filament);
	c->extract_params_from_command_string();
	p_p = c->get_params();
	execute_G1(p_p);
}
