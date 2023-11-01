/*
 * Command.cpp
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#include "Command.h"
#include <cstring>
#include <stdlib.h>


Command::Command() {
	params.x.param_value = 0.0f;
	params.x.is_param_valid = false;
	params.y.param_value = 0.0f;
	params.y.is_param_valid = false;
	params.z.param_value = 0.0f;
	params.z.is_param_valid = false;
	params.e.param_value = 0.0f;
	params.e.is_param_valid = false;
	params.f.param_value = 0.0f;
	params.f.is_param_valid = false;
	params.s.param_value = 0.0f;
	params.s.is_param_valid = false;
}

Command::~Command() {

}

void Command::set_code_and_param_string(char* full_instruction_line) {
	if (!is_instruction_comment(full_instruction_line) && !is_instruction_empty(full_instruction_line)) {
		uint32_t pos = 0;
		while(full_instruction_line[pos] != ' ') {
			pos++;
		}
		memset(params.command_code, 0, MAX_COMMAND_SIZE);
		memset(params_string, 0, MAX_PARAM_SIZE);
		strncpy(params.command_code, full_instruction_line, pos);
		//TODO ITT ellenőrizni, hogy a +1 jól működik-e!!!
		strncpy(params_string, full_instruction_line + pos + 1, find_instruction_end_char(full_instruction_line) - pos);
	}
}

void Command::extract_params_from_command_string() {
	char* token = strtok(params_string, " ");
	invalidate_all_params();

    while (token != nullptr) {
        char param_key = token[0];
        if (param_key == 'X') {
        	//TODO Itt gond lehet a +1-el, nem tudom, extrán figyelni rá teszteléskor!!!
            params.x.param_value = strtof(token + 1, nullptr);
            params.x.is_param_valid = true;
        } else if (param_key == 'Y') {
            params.y.param_value = strtof(token + 1, nullptr);
            params.y.is_param_valid = true;
        } else if (param_key == 'Z') {
            params.z.param_value = strtof(token + 1, nullptr);
            params.z.is_param_valid = true;
        } else if (param_key == 'E') {
            params.e.param_value = strtof(token + 1, nullptr);
            params.e.is_param_valid = true;
        } else if (param_key == 'F') {
            params.f.param_value = strtof(token + 1, nullptr);
            params.f.is_param_valid = true;
        } else if (param_key == 'S') {
            params.s.param_value = strtof(token + 1, nullptr);
            params.s.is_param_valid = true;
        } else {
        	//ERROR: Unknown parameter
        }
        token = strtok(nullptr, " ");
    }
}

void Command::invalidate_all_params() {
	params.x.is_param_valid = false;
	params.y.is_param_valid = false;
	params.z.is_param_valid = false;
	params.e.is_param_valid = false;
	params.f.is_param_valid = false;
	params.s.is_param_valid = false;
}

int32_t Command::find_instruction_end_char(char* instruction) {
	int32_t pos = 0;
    while (instruction[pos] != '\0') {
    	if (instruction[pos] == '\n' || instruction[pos] == ';') {
            return pos;
        }
        pos++;
    }
    return -1;
}

bool Command::is_instruction_comment(char* instruction) {
	if(instruction[0] == ';') {
		return true;
	}
	return false;
}

bool Command::is_instruction_empty(char* instruction) {
	if(instruction[0] == '\n') {
		return true;
	}
	return false;
}

char* Command::get_command_code() {
	return params.command_code;
}

Possible_params* Command::get_params() {
	return &params;
}

