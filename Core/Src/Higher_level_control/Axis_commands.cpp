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
	current_feedrate = 1.0f;

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
	//Home axes
	uint8_t is_going_home[NUM_OF_DESCARTES_AXES] = {0};
	if (command->x == INVALID_COMMAND_PARAM && command->y == INVALID_COMMAND_PARAM && command->z == INVALID_COMMAND_PARAM) {
		home_axis(axis_X, is_going_home, 0);
		home_axis(axis_Y, is_going_home, 1);
		home_axis(axis_Z, is_going_home, 2);
	}
	else {
		if (command->x != INVALID_COMMAND_PARAM) home_axis(axis_X, is_going_home, 0);
		if (command->y != INVALID_COMMAND_PARAM) home_axis(axis_Y, is_going_home, 1);
		if (command->z != INVALID_COMMAND_PARAM) home_axis(axis_Z, is_going_home, 2);
	}
	reset_motors_home_params(is_going_home);
	HAL_TIM_Base_Start_IT(motor_timer);
}

void execute_G90(Command_struct* command) {
	//Set absolute positioning
	//The current software version only supports absolute positioning, so no action needed
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_G92(Command_struct* command) {
	//Modify the gives axis's position parameter
	if (command->x != INVALID_COMMAND_PARAM) {
		axis_X->update_position(command->x);
	}
	if (command->y != INVALID_COMMAND_PARAM) {
		axis_Y->update_position(command->y);
	}
	if (command->z != INVALID_COMMAND_PARAM) {
		axis_Z->update_position(command->z);
	}
	if (command->e != INVALID_COMMAND_PARAM) {
		axis_E->update_position(command->e);
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
	Axis* axes[] = {axis_X, axis_Y, axis_Z, axis_E};
	float axis_params[] = {command->x, command->y, command->z, command->e};

	float travel_distances[NUM_OF_AXES] = {0.0f};
	uint8_t travel_directions[NUM_OF_AXES] = {0};
	float accel[NUM_OF_AXES] = {0.0f};
	uint32_t step_num_required[NUM_OF_AXES] = {0};

	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (axis_params[i] != INVALID_COMMAND_PARAM) {
			travel_directions[i] = axes[i]->calculate_dir(axis_params[i]);
			axes[i]->get_motor()->change_stepper_dir_pin(travel_directions[i]);
			travel_distances[i] = abs(axis_params[i] - axes[i]->get_axis_pos());
			step_num_required[i] = axes[i]->calculate_step_num(axis_params[i]);
			axes[i]->update_position(axis_params[i]);
		}
	}

	if (command->f != INVALID_COMMAND_PARAM) current_feedrate = command->f;

	float max_acc_time = get_max_accel_time(axes, travel_distances);
	adjust_accel_to_time(axes, accel, travel_distances, max_acc_time);
	reset_motor_linear_acc_params(step_num_required, accel, travel_directions);
	HAL_TIM_Base_Start_IT(motor_timer);
}

void home_axis(Axis* axis, uint8_t* is_going_home, uint32_t idx) {
    ((Descartes_Axis*)axis)->get_motor()->change_stepper_dir_pin(((Descartes_Axis*)axis)->get_limit_switch_dir());
    axis->set_home_position();
    is_going_home[idx] = 1;
}

float get_max_accel_time(Axis* axes[], float* travel_distances) {
	float max_accel_time = 0.0f;
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		float acc_time = axes[i]->time_to_reach_speed_max_accel(current_feedrate, travel_distances[i]);
		if (acc_time > max_accel_time) {
			max_accel_time = acc_time;
		}
	}
	return max_accel_time;
}

float get_max_time_trapezoid(Axis* axes[], float* acc_dist, float* full_dist, float* max_acc_to_full_dist_ratio, float* max_const_time) {
	float max_full_time = 0.0f;
	float max_acc_time = 0.0f;
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (full_dist[i] <= 0.0f) {
			continue;
		}

		float used_speed_mm_s = current_feedrate / 60.0f;
		if (axes[i]->get_max_speed() < current_feedrate) {
			used_speed_mm_s = axes[i]->get_max_speed() / 60.0;
		}
		acc_dist[i] = (used_speed_mm_s) * (used_speed_mm_s) / (2.0f * axes[i]->get_max_accel());
		if (2.0f * acc_dist[i] > full_dist[i]) {
			acc_dist[i] = full_dist[i] / 2.0f;
		}
		float acc_time = (used_speed_mm_s) / axes[i]->get_max_speed();
		float const_time = (full_dist[i] - 2 * acc_dist[i]) / used_speed_mm_s;
		float full_time = 2.0f * acc_time + const_time;
		if (full_time > max_full_time) {
			max_acc_time = acc_time;
			*max_acc_to_full_dist_ratio = acc_dist[i] / full_dist[i];
			*max_const_time = const_time;
		}
	}
	return max_acc_time;
}

void adjust_accel_to_time(Axis* axes[], float* accel, float* travel_distances, float accel_time) {
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		accel[i] = axes[i]->acceleration_for_time_and_distance(accel_time, travel_distances[i]);
	}
}

void adjust_accel_to_time_trapezoid(Axis* axes[], float accel[], float acc_dist[], float travel_distances[], float accel_time, float used_v0[], float max_acc_to_full_dist_ratio) {
	for (uint32_t i = 0; i < NUM_OF_AXES; i++) {
		if (travel_distances[i] <= 0.0f) {
			continue;
		}

		acc_dist[i] = max_acc_to_full_dist_ratio * travel_distances[i];
		accel[i] = 2.0f * acc_dist[i] / (accel_time * accel_time);
	}
}

