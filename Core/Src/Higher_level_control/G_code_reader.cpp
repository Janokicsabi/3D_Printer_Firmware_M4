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

//TODO Majd arra nagyon figyelni, hogy a fájl végén mi történik, hogy olvassuk!!!
void task_fill_message_queue(void* sd_card) {
	Command* command = new Command();
	char full_command_line[MAX_COMMAND_LENGTH];

	while(1) {
		memset(full_command_line, 0, MAX_COMMAND_LENGTH);
		((SD_card*)sd_card)->read_one_line(full_command_line, MAX_COMMAND_LENGTH);
		command->set_code_and_param_string(full_command_line);
		command->extract_params_from_command_string();
		Possible_params* current_params = command->get_params();
		while(xQueueSend(queue_command, (void*)current_params, 0) != pdPASS) {
			vTaskDelay(10000);
		}
		vTaskDelay(100);
	}
}
