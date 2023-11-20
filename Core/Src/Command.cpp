/*
 * Command.cpp
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#include "Command.h"
#include <cstring>
#include <stdlib.h>
#include <ctype.h>


Command::Command() {
	this->invalidate_all_params();
}

Command::~Command() {

}

bool Command::set_code_and_param_string(char* full_instruction_line) {
	if (!is_instruction_executable(full_instruction_line)) {
		return false;
	}

	uint32_t inst_code_end_pos = find_command_code_end_char(full_instruction_line);
	uint32_t param_start_pos = find_param_start_char(full_instruction_line);
	uint32_t end_pos = find_instruction_end_char(full_instruction_line);
	memset(params.command_code, 0, MAX_COMMAND_SIZE);
	memset(params_string, 0, MAX_PARAM_SIZE);
	strncpy(params.command_code, full_instruction_line, inst_code_end_pos);
	strncpy(params_string, full_instruction_line + param_start_pos, end_pos - param_start_pos);
	return true;
}

int32_t Command::find_param_start_char(char* full_instruction_line) {
    int32_t i = 0;
    while (full_instruction_line[i] != '\0') {
        if (full_instruction_line[i] == ' ') {
        	return i;
        }
        i++;
    }
    return -1;
}

uint32_t Command::find_command_code_end_char(char* full_instruction_line) {
    uint32_t i = 0;
    while (isalnum(full_instruction_line[i]) && full_instruction_line[i] != '\0') {
        i++;
    }
    return i;
}

void Command::extract_params_from_command_string() {
	invalidate_all_params();
	char* token = strtok(params_string, " ");

    while (token != nullptr) {
        char param_key = token[0];
        if (param_key == 'X') params.x = strtof(token + 1, nullptr);
        if (param_key == 'Y') params.y = strtof(token + 1, nullptr);
        if (param_key == 'Z') params.z = strtof(token + 1, nullptr);
        if (param_key == 'E') params.e = strtof(token + 1, nullptr);
        if (param_key == 'F') params.f = strtof(token + 1, nullptr);
        if (param_key == 'S') params.s = strtof(token + 1, nullptr);
        token = strtok(nullptr, " ");
    }
}

void Command::invalidate_all_params() {
	params.x = INVALID_COMMAND_PARAM;
	params.y = INVALID_COMMAND_PARAM;
	params.z = INVALID_COMMAND_PARAM;;
	params.e = INVALID_COMMAND_PARAM;
	params.f = INVALID_COMMAND_PARAM;
	params.s = INVALID_COMMAND_PARAM;
}

int32_t Command::find_instruction_end_char(char* full_instruction_line) {
    char command_end_chars[] = {';', '\n', '\0'};
    char* result = strpbrk(full_instruction_line, command_end_chars);
    if (result != nullptr) {
        return result - full_instruction_line - 1;
    } else {
        return -1;
    }
}

bool Command::is_instruction_comment(char* instruction) {
	if(instruction[0] == ';') {
		return true;
	}
	return false;
}

bool Command::is_instruction_executable(char* instruction) {
	if (instruction[0] == 'G' || instruction[0] == 'M') {
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

Command_struct* Command::get_params() {
	return &params;
}

