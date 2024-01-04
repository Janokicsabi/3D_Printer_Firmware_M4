/*
 * Command.h
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#ifndef SRC_COMMAND_H_
#define SRC_COMMAND_H_

#include "main.h"
#include <math.h>

#define MAX_COMMAND_SIZE	5
#define MAX_PARAM_SIZE		70

#define INVALID_COMMAND_PARAM 		INFINITY


typedef struct {
	char command_code[MAX_COMMAND_SIZE];
	float x;
	float y;
	float z;
	float e;
	float f;
	float s;
}Command_struct;

using namespace std;


class Command {
private:
	char params_string[MAX_PARAM_SIZE];
	Command_struct params;

	void invalidate_all_params();
	bool is_instruction_comment(char* instruction);
	bool is_instruction_empty(char* instruction);
	int32_t find_instruction_end_char(char* instruction);
	int32_t find_param_start_char(char* full_instruction_line);
	uint32_t find_command_code_end_char(char* full_instruction_line);
	bool is_instruction_executable(char* full_instruction_line);

public:
	Command();
	virtual ~Command();

	bool set_code_and_param_string(char* full_command_line);
	void extract_params_from_command_string();
	char* get_command_code();
	Command_struct* get_params();
};

#endif /* SRC_COMMAND_H_ */
