/*
 * Axis_commands.h
 *
 *  Created on: 2023. okt. 16.
 *      Author: janok
 */

#ifndef SRC_AXISCOMMANDS_H_
#define SRC_AXISCOMMANDS_H_

#include <Stepper.h>
#include <Axis.h>
#include <Descartes_Axis.h>
#include <Command.h>
#include "G_code_reader.h"


void axis_commands_init(TIM_HandleTypeDef* timer, Descartes_Axis* axis_x, Descartes_Axis* axis_y, Descartes_Axis* axis_z, Axis* axis_e);
void task_axis_control(void* param);
void execute_G1(Command_struct* command);
void execute_G21(Command_struct* command);
void execute_G28(Command_struct* command);
void execute_G90(Command_struct* command);
void execute_G92(Command_struct* command);
void execute_M82(Command_struct* command);
void execute_M84(Command_struct* command);
void task_wait_for_stepper_stop(void* params);
int32_t get_command_index(char* code);
void execute_axis_move_command(Command_struct* new_axis_params);

#endif /* SRC_AXISCOMMANDS_H_ */
