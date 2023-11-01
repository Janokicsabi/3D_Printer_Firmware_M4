/*
 * Axis.cpp
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#include "Axis.h"
#include <cmath>


Axis::Axis(Motor* motor, Limit_switch* limit_switch, const float workspace_frame_offset, const float axis_endpoint,
		const float axis_pitch, const float pitch_num_in_one_rotation, const uint8_t limit_switch_dir) :
		workspace_frame_offset{workspace_frame_offset}, axis_endpoint_in_ws_frame{axis_endpoint}, axis_pitch{axis_pitch},
		pitch_num_in_one_rotation{pitch_num_in_one_rotation}, displacement_per_microstep {
			(MOTOR_BASIC_STEP_DEGREE / FULL_ROTATION_DEGREE) * (1.0 / motor->get_microstep_devider()) * pitch_num_in_one_rotation * axis_pitch},
		limit_switch_dir{limit_switch_dir} {
	this->motor = motor;
	this->limit_switch = limit_switch;
	this->pos_in_ws_frame = 0;
}

Axis::~Axis() {

}

const uint32_t Axis::calculate_step_num(float new_pos) {
	new_pos = saturate_position(new_pos);
	float distance = fabs(new_pos - this->pos_in_ws_frame);
	uint32_t step_num = (uint32_t)round(distance / this->displacement_per_microstep);
	return step_num;
}

const uint8_t Axis::calculate_dir(float new_pos) {
	if (this->pos_in_ws_frame < new_pos) {
		return 1;
	} else {
		return 0;
	}
}

float Axis::saturate_position(float new_pos) {
	uint8_t dir = calculate_dir(new_pos);
	if (dir != limit_switch_dir) {
		if (new_pos > axis_endpoint_in_ws_frame) {
			new_pos = axis_endpoint_in_ws_frame;
		} else if (new_pos < (0.0 - workspace_frame_offset)) {
			new_pos = (0.0 - workspace_frame_offset);
		}
	}
	return new_pos;
}

const char Axis::get_axis_name() {
	return this->axis_name;
}

const float Axis::get_axis_pos() {
	return this->pos_in_ws_frame;
}

Motor* Axis::get_motor() {
	return this->motor;
}

const float Axis::get_one_step_displacement() {
	return this->displacement_per_microstep;
}

void Axis::control_axis(float move_speed, float new_pos, bool is_feedrate_const) {
	//The position is the same, so no motor movement required
	if (!this->is_position_changed(new_pos)) {
		return;
	}

	this->motor->set_motor_speed(move_speed, this->displacement_per_microstep);
	uint8_t dir = this->calculate_dir(new_pos);
	uint32_t step_num = this->calculate_step_num(new_pos);
	//TODO Visszaállítani
	//if (is_feedrate_const) {
	this->motor->motor_move_const(step_num, dir);
	update_position(new_pos);
	//}
	//else {
	//	uint32_t previous_timer_period = motor_map[motor_num]->calculate_motor_timer_period_from_speed(prev_motor_parameters.f, motor_map[motor_num]->get_displacement_per_microstep());
	//	uint32_t feedrate_accel_time_diff = (timer_period - previous_timer_period) / (float)step_num;
	//	motor_map[motor_num]->motor_move_accel(step_num, dir, feedrate_accel_time_diff);
	//}
}

void Axis::home_axis(float move_speed) {
	const uint32_t MOVE_MAX_STEPS = 0xFFFFFFFF; 	//Biggest possible step num. Start moving, and it will be stopped by the limit switch
	this->motor->set_motor_speed(move_speed, this->displacement_per_microstep);
	this->motor->motor_move_const(MOVE_MAX_STEPS, this->limit_switch_dir);
	//Hence the 0 position is in the workspace frame, the limit switch is in negative direction
	//and shifted with the workspace_frame_offset
	update_position(0.0 - workspace_frame_offset);
}

const bool Axis::is_position_changed(float new_pos) {
	if (new_pos != this->pos_in_ws_frame) {
		return true;
	}
	else {
		return false;
	}
}

void Axis::update_position(float new_pos) {
	this->pos_in_ws_frame = new_pos;
}

