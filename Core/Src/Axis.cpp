/*
 * Axis.cpp
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#include "Axis.h"
#include <cmath>


Axis::Axis(Motor* motor, const float full_rotation_displacement) {
	this->motor = motor;
	this->pos_in_ws_frame = 0.0f;

	float full_step_degrees = motor->get_full_step_degree();
	float microstep_devider = motor->get_microstep_devider();
	this->displacement_per_microstep = (full_step_degrees / FULL_ROTATION_DEGREE) * (1.0 / microstep_devider) * full_rotation_displacement;
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

const bool Axis::is_position_changed(float new_pos) {
	if (new_pos != this->pos_in_ws_frame) {
		return true;
	} else {
		return false;
	}
}

void Axis::update_position(float new_pos) {
	this->pos_in_ws_frame = new_pos;
}

float Axis::saturate_position(float new_pos) {
	return new_pos;
}

Limit_switch* Axis::get_limit_switch() {
	return nullptr;
}


