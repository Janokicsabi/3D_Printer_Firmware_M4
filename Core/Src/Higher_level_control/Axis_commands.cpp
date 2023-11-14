/*
 * Axis_commands.cpp
 *
 *  Created on: 2023. okt. 16.
 *      Author: janok
 */

#include <Axis_commands.h>
#include "Motor.h"
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
#include <string>
#include <string.h>
using namespace std;

static float current_feedrate;
static Descartes_Axis* axis_X;
static Descartes_Axis* axis_Y;
static Descartes_Axis* axis_Z;
static Axis* axis_E;

extern TIM_HandleTypeDef htim16;

extern EventGroupHandle_t command_state;


void axis_commands_init(Descartes_Axis* axis_x, Descartes_Axis* axis_y, Descartes_Axis* axis_z, Axis* axis_e) {
	//TODO Átgondolni, hogy ez itt legyen-e
	Motor::enable_motors();
	current_feedrate = 0.0f;

	axis_X = axis_x;
	axis_Y = axis_y;
	axis_Z = axis_z;
	axis_E = axis_e;
}

void execute_G1(Command_struct* command) {
	//Move end effector from point to point in a linear line
	execute_axis_move_command(command);
	xEventGroupWaitBits(Motor::event_motor_standing, MOTOR_X_FINISHED | MOTOR_Y_FINISHED |
			  MOTOR_Z_FINISHED | MOTOR_E_FINISHED, pdFALSE, pdTRUE, portMAX_DELAY);
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_G21(Command_struct* command) {
	//Set all units to millimeter
	//The current software version only supports units in mm, so no conversion is necessary
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_G28(Command_struct* command) {
	//Home all axis
	const float HOME_MOVE_SPEED = 2000;
	axis_X->home_axis(HOME_MOVE_SPEED);
	xEventGroupWaitBits(Motor::event_motor_standing, MOTOR_X_FINISHED | MOTOR_Y_FINISHED |
			  MOTOR_Z_FINISHED | MOTOR_E_FINISHED, pdFALSE, pdTRUE, portMAX_DELAY);
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
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
	Motor::disable_motors();
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);
}

void execute_axis_move_command(Command_struct* command) {
	//TODO speed számítás ellenőriz
	//TODO Probléma: gyorsítás esetén nem lesz jó, ha nem maximális feed rate sebességgel mozgott
	float current_x_pos = axis_X->get_axis_pos();
	float current_y_pos = axis_Y->get_axis_pos();
	float current_z_pos = axis_Z->get_axis_pos();
	float current_e_pos = axis_E->get_axis_pos();

	float dx, dy, dz, de;
	(command->x.is_param_valid) ? dx = abs(command->x.param_value - current_x_pos) : dx = 0.0f;
	(command->y.is_param_valid) ? dy = abs(command->y.param_value - current_y_pos) : dy = 0.0f;
	(command->z.is_param_valid) ? dz = abs(command->z.param_value - current_z_pos) : dz = 0.0f;
	(command->e.is_param_valid) ? de = abs(command->e.param_value - current_e_pos) : de = 0.0f;

	if (command->f.is_param_valid) {
		current_feedrate = command->f.param_value;
	}

	//TODO Majd elmozdítani!!!
	float a_max = 100;			//[mm/s^2]
	float a_used = a_max;
	float max_dist = fmax(fmax(dx, dy), dz);
	float acc_time = sqrt(max_dist / a_max);
	//Time to reach the maximum speed (feed rate)
	float feed_rate_mm_s = current_feedrate / 60.0f;
	float time_to_v_max = feed_rate_mm_s / a_max;
	if (time_to_v_max < acc_time) {
		a_used = (feed_rate_mm_s * feed_rate_mm_s) / max_dist;
		acc_time = sqrt(max_dist / a_used);
	}

	float a_x = dx / (acc_time * acc_time);
	float a_y = dy / (acc_time * acc_time);
	float a_z = dz / (acc_time * acc_time);
	float a_e = de / (acc_time * acc_time);
	float acc[NUM_OF_AXES] = {a_x, a_y, a_z, a_e};

	uint32_t step_num_required[NUM_OF_AXES];
	dx != 0 ? step_num_required[0] = axis_X->calculate_step_num(command->x.param_value) : step_num_required[0] = 0;
	dy != 0 ? step_num_required[1] = axis_Y->calculate_step_num(command->y.param_value) : step_num_required[1] = 0;
	dz != 0 ? step_num_required[2] = axis_Z->calculate_step_num(command->z.param_value) : step_num_required[2] = 0;
	de != 0 ? step_num_required[3] = axis_E->calculate_step_num(command->e.param_value) : step_num_required[3] = 0;

	if (dx!=0) axis_X->get_motor()->change_motor_dir_pin(axis_X->calculate_dir(command->x.param_value));
	if (dy!=0) axis_Y->get_motor()->change_motor_dir_pin(axis_Y->calculate_dir(command->y.param_value));
	if (dz!=0) axis_Z->get_motor()->change_motor_dir_pin(axis_Z->calculate_dir(command->z.param_value));
	if (de!=0) axis_E->get_motor()->change_motor_dir_pin(axis_E->calculate_dir(command->e.param_value));

	reset_motor_linear_acc_params(step_num_required, acc, acc_time);

	HAL_TIM_Base_Start_IT(&htim16);
}

