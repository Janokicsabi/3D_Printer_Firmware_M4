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
int32_t get_command_index(char* code);
void execute_axis_move_command(Command_struct* new_axis_params);
void home_axis(Axis* axis, uint8_t* is_going_home, uint32_t idx);
float get_max_accel_time(Axis* axes[], float* travel_distances);
void adjust_accel_to_time(Axis* axes[], float* accel, float* travel_distances, float accel_time);

float get_max_time_trapezoid(Axis* axes[], float* acc_dist, float* full_dist, float* max_acc_to_full_dist_ratio, float* max_const_time);
void adjust_accel_to_time_trapezoid(Axis* axes[], float accel[], float acc_dist[], float travel_distances[], float accel_time, float used_v0[], float max_acc_to_full_dist_ratio);

#endif /* SRC_AXISCOMMANDS_H_ */
