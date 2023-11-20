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

static char full_command_line[MAX_COMMAND_LENGTH];


void task_fill_message_queue(void* sd_card) {
	while(1) {
		if (f_eof(((SD_card*)sd_card)->get_file()) != 0) {
			//TODO Ide esetleg valami flag, hogy jelezze, vége az egésznek.
			//Fontos, hogy ezután még le kell futniuk a queue-ba helyezett üzeneteknek!
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
