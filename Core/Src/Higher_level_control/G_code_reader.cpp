/*
 * G_code_reader.cpp
 *
 *  Created on: 2023. okt. 16.
 *      Author: janok
 */

#include <G_code_reader.h>

#include <cstring>
#include <Command.h>

#define MAX_COMMAND_LENGTH		(MAX_COMMAND_SIZE + MAX_PARAM_SIZE)
extern xQueueHandle queue_command;
extern EventGroupHandle_t event_command_read_ready;

static char full_command_line[MAX_COMMAND_LENGTH];


void task_g_code_interpreter(void* sd_card) {
	while(1) {
		//Check for the eof (end of file) character
		if (f_eof(((SD_card*)sd_card)->get_file()) != 0) {
			//It reached the end of file, no more instructions remain
			xEventGroupSetBits(event_command_read_ready, COMMAND_READ_FINISHED);
			vTaskDelete(NULL);
		}

		Command command;
		memset(full_command_line, 0, MAX_COMMAND_LENGTH);
		if(((SD_card*)sd_card)->read_one_line(full_command_line, MAX_COMMAND_LENGTH)) {
			if (command.set_code_and_param_string(full_command_line)) {
				command.extract_params_from_command_string();
				Command_struct* current_params = command.get_params();
				while(xQueueSend(queue_command, (void*)current_params, 0) != pdPASS) {
					vTaskDelay(1000);
				}
			}
		}
		vTaskDelay(50);
	}
}
