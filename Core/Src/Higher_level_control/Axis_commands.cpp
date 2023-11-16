/*
 * Axis_commands.cpp
 *
 *  Created on: 2023. okt. 16.
 *      Author: janok
 */

#include <Axis_commands.h>

#include "Axis.h"
#include "Descartes_Axis.h"
#include "task_creator.h"
#include "G_code_reader.h"
#include "Command_control.h"
#include "cpp_callback_wrap.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

#include <cmath>
#include <functional>
#include <map>
#include <math.h>
#include <Stepper.h>
#include <string>
#include <string.h>
using namespace std;

static float current_feedrate;
static Descartes_Axis* axis_X;
static Descartes_Axis* axis_Y;
static Descartes_Axis* axis_Z;
static Axis* axis_E;

static TIM_HandleTypeDef* motor_timer;

extern EventGroupHandle_t command_state;


void axis_commands_init(TIM_HandleTypeDef* timer, Descartes_Axis* axis_x, Descartes_Axis* axis_y, Descartes_Axis* axis_z, Axis* axis_e) {
	//TODO Átgondolni, hogy ez itt legyen-e
	Stepper::enable_steppers();
	current_feedrate = 0.0f;

	axis_X = axis_x;
	axis_Y = axis_y;
	axis_Z = axis_z;
	axis_E = axis_e;
	motor_timer = timer;
}

void execute_G1(Command_struct* command) {
	//Move end effector from point to point in a linear line
	execute_axis_move_command(command);
}

void execute_G21(Command_struct* command) {
	//Set all units to millimeter
	//The current software version only supports units in mm, so no conversion is necessary
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_G28(Command_struct* command) {
	//Home all axis
	//TODO EZT MEGCSINÁLNI
	reset_motors_home_params();
	axis_X->get_motor()->change_stepper_dir_pin(axis_X->get_limit_switch_dir());
	axis_Y->get_motor()->change_stepper_dir_pin(axis_Y->get_limit_switch_dir());
	axis_Z->get_motor()->change_stepper_dir_pin(axis_Z->get_limit_switch_dir());
	HAL_TIM_Base_Start_IT(motor_timer);
}

void execute_G90(Command_struct* command) {
	//Set absolute positioning
	//The current software version only supports absolute positioning, so no action needed
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_G92(Command_struct* command) {
	//Modify the gives axis's position parameter
	if (command->x.is_param_valid) {
		axis_X->update_position(command->x.param_value);
	}
	if (command->y.is_param_valid) {
		axis_Y->update_position(command->y.param_value);
	}
	if (command->z.is_param_valid) {
		axis_Z->update_position(command->z.param_value);
	}
	if (command->e.is_param_valid) {
		axis_E->update_position(command->e.param_value);
	}
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_M82(Command_struct* command) {
	//Set the extruder to absolute positioning mode
	//The current software version only supports absolute positioning, so no action needed
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_M84(Command_struct* command) {
	//Disable motors
	Stepper::disable_steppers();
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_axis_move_command(Command_struct* command) {
	float dx, dy, dz, de;
	(command->x.is_param_valid) ? dx = abs(command->x.param_value - axis_X->get_axis_pos()) : dx = 0.0f;
	(command->y.is_param_valid) ? dy = abs(command->y.param_value - axis_Y->get_axis_pos()) : dy = 0.0f;
	(command->z.is_param_valid) ? dz = abs(command->z.param_value - axis_Z->get_axis_pos()) : dz = 0.0f;
	(command->e.is_param_valid) ? de = abs(command->e.param_value - axis_E->get_axis_pos()) : de = 0.0f;

	uint32_t step_num_required[NUM_OF_AXES];
	dx != 0 ? step_num_required[0] = axis_X->calculate_step_num(command->x.param_value) : step_num_required[0] = 0;
	dy != 0 ? step_num_required[1] = axis_Y->calculate_step_num(command->y.param_value) : step_num_required[1] = 0;
	dz != 0 ? step_num_required[2] = axis_Z->calculate_step_num(command->z.param_value) : step_num_required[2] = 0;
	de != 0 ? step_num_required[3] = axis_E->calculate_step_num(command->e.param_value) : step_num_required[3] = 0;

	uint8_t dir_x, dir_y, dir_z, dir_e;
	dir_x = axis_X->calculate_dir(command->x.param_value);
	dir_y = axis_Y->calculate_dir(command->y.param_value);
	dir_z = axis_Z->calculate_dir(command->z.param_value);
	dir_e = axis_E->calculate_dir(command->e.param_value);
	uint8_t dirs[] = {dir_x, dir_y, dir_z, dir_e};
	if (dx!=0) axis_X->get_motor()->change_stepper_dir_pin(dir_x);
	if (dy!=0) axis_Y->get_motor()->change_stepper_dir_pin(dir_y);
	if (dz!=0) axis_Z->get_motor()->change_stepper_dir_pin(dir_z);
	if (de!=0) axis_E->get_motor()->change_stepper_dir_pin(dir_e);

	if (command->f.is_param_valid) {
		current_feedrate = command->f.param_value;
	}

	Axis* axes[] = {axis_X, axis_Y, axis_Z, axis_E};
	float travel_distances[] = {dx, dy, dz, de};
	float max_acc_time = 0;
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		float acc_time = axes[i]->time_to_reach_speed_max_accel(current_feedrate, travel_distances[i]);
		if (acc_time > max_acc_time) {
			max_acc_time = acc_time;
		}
	}
	float acc[NUM_OF_AXES];
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		acc[i] = axes[i]->acceleration_for_time_and_distance(max_acc_time, travel_distances[i]);
	}

	reset_motor_linear_acc_params(step_num_required, acc, max_acc_time, dirs);
	HAL_TIM_Base_Start_IT(motor_timer);
}

