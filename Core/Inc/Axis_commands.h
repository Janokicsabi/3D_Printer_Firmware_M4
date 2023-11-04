/*
 * Axis_commands.h
 *
 *  Created on: 2023. okt. 16.
 *      Author: janok
 */

#ifndef SRC_AXISCOMMANDS_H_
#define SRC_AXISCOMMANDS_H_

#include <Motor.h>
#include <Axis.h>
#include <Command.h>
#include "G_code_reader.h"


void axis_commands_init(Axis* axis_x, Axis* axis_y, Axis* axis_z, Axis* axis_e);
void task_axis_control(void* param);
void execute_G1(Possible_params* command);
void execute_G21(Possible_params* command);
void execute_G28(Possible_params* command);
void execute_G90(Possible_params* command);
void execute_G92(Possible_params* command);
void execute_M82(Possible_params* command);
void execute_M84(Possible_params* command);
int32_t get_command_index(char* code);
void execute_axis_move_command(Possible_params* new_axis_params);

#endif /* SRC_AXISCOMMANDS_H_ */
