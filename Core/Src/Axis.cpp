/*
 * Axis.cpp
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#include "Axis.h"
#include <cmath>


Axis::Axis(Stepper* motor, const float full_rotation_displacement, float max_acc, float max_speed) {
	this->motor = motor;
	this->pos_in_ws_frame = 0.0f;
	this->max_acc = max_acc;
	this->max_speed = max_speed;

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

Stepper* Axis::get_motor() {
	return this->motor;
}

const float Axis::get_one_step_displacement() {
	return this->displacement_per_microstep;
}

void Axis::control_axis_pwm(float move_speed, float new_pos, bool is_feedrate_const) {
	//The position is the same, so no motor movement required
	if (!this->is_position_changed(new_pos)) {
		return;
	}

	this->motor->set_motor_speed(move_speed, this->displacement_per_microstep);
	uint8_t dir = this->calculate_dir(new_pos);
	uint32_t step_num = this->calculate_step_num(new_pos);
	this->motor->motor_move_const(step_num, dir);
	update_position(new_pos);
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

float Axis::time_to_reach_speed_max_accel(float desired_speed_mm_p_min, float distance_to_move) {
	if (desired_speed_mm_p_min > max_speed) {
		desired_speed_mm_p_min = max_speed;
	}
	float desired_speed = convert_feedrate_to_mm_p_s(desired_speed_mm_p_min);
	float acc_time = sqrt(distance_to_move / max_acc);
	float time_to_reach_desired_speed = desired_speed / max_acc;
	if (time_to_reach_desired_speed < acc_time) {
		float a_used = pow(desired_speed, 2) / distance_to_move;
		acc_time = sqrt(distance_to_move / a_used);
	}
	return acc_time;
}


float Axis::acceleration_for_time_and_distance(float acc_time, float distance_to_move) {
	return distance_to_move / (acc_time * acc_time);
}

float Axis::convert_feedrate_to_mm_p_s(float feedrate_mm_p_min) {
	return feedrate_mm_p_min / SECONDS_IN_A_MINUTE;
}


