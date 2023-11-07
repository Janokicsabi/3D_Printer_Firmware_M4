/*
 * Command.h
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#ifndef SRC_COMMAND_H_
#define SRC_COMMAND_H_

#include "main.h"
#include <map>

#define MAX_COMMAND_SIZE	5
#define MAX_PARAM_SIZE		70

typedef struct {
	float param_value;
	bool is_param_valid = false;
}Command_Param;

typedef struct {
	char command_code[MAX_COMMAND_SIZE];
	Command_Param x;
	Command_Param y;
	Command_Param z;
	Command_Param e;
	Command_Param f;
	Command_Param s;
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

public:
	Command();
	virtual ~Command();

	bool set_code_and_param_string(char* full_command_line);
	void extract_params_from_command_string();
	char* get_command_code();
	Command_struct* get_params();
};

#endif /* SRC_COMMAND_H_ */
