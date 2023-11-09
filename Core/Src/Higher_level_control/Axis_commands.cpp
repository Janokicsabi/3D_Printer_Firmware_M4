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
static Axis* axis_Y;
static Axis* axis_Z;
static Axis* axis_E;

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

	float end_eff_travel_dist = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
	if (end_eff_travel_dist == 0 && command->e.is_param_valid) {
		end_eff_travel_dist = command->e.param_value;
	}
	//Move time in [min] (because feed rate is in [mm/min])
	float move_time = fabs(end_eff_travel_dist / current_feedrate);

	//bool is_feedrate_const = (prev_axis_parameters.f == new_axis_parameters.f);
	bool is_feedrate_const = true;

	if (command->x.is_param_valid) {
		float move_speed_x = dx / move_time;
		axis_X->control_axis(move_speed_x, command->x.param_value, is_feedrate_const);
	}
	if (command->y.is_param_valid) {
		float move_speed_y = dy / move_time;
		axis_Y->control_axis(move_speed_y, command->y.param_value, is_feedrate_const);
	}
	if (command->z.is_param_valid) {
		float move_speed_z = dz / move_time;
		axis_Z->control_axis(move_speed_z, command->z.param_value, is_feedrate_const);
	}
	if (command->e.is_param_valid) {
		float move_speed_e = de / move_time;
		axis_E->control_axis(move_speed_e, command->e.param_value, is_feedrate_const);
	}
}

