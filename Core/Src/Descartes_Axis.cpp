/*
 * Descartes_Axis.cpp
 *
 *  Created on: Nov 6, 2023
 *      Author: janok
 */

#include "Descartes_Axis.h"

Descartes_Axis::Descartes_Axis(Motor* motor, Limit_switch* limit_switch, const float workspace_frame_offset, const float axis_endpoint,
		const float full_rotation_displacement, const uint8_t limit_switch_dir) : Axis(motor, full_rotation_displacement),
		limit_switch_dir {limit_switch_dir}, workspace_frame_offset {workspace_frame_offset}, axis_length {axis_length}, limit_switch {limit_switch} {

}

Descartes_Axis::~Descartes_Axis() {

}

float Descartes_Axis::saturate_position(float new_pos) {
	uint8_t dir = calculate_dir(new_pos);
	if (dir != limit_switch_dir) {
		if (new_pos > (axis_length - workspace_frame_offset)) {
			new_pos = (axis_length - workspace_frame_offset);
		} else if (new_pos < (0.0 - workspace_frame_offset)) {
			new_pos = (0.0 - workspace_frame_offset);
		}
	}
	return new_pos;
}

Limit_switch* Descartes_Axis::get_limit_switch() {
	return this->limit_switch;
}

void Descartes_Axis::home_axis(float move_speed) {
	const uint32_t MOVE_MAX_STEPS = 0xFFFFFFFF; 	//Biggest possible step num. Start moving, and it will be stopped by the limit switch
	this->motor->set_motor_speed(move_speed, this->displacement_per_microstep);
	this->motor->motor_move_const(MOVE_MAX_STEPS, this->limit_switch_dir);
	//Hence the 0 position is in the workspace frame, the limit switch is in negative direction
	//and shifted with the workspace_frame_offset
	update_position(0.0 - workspace_frame_offset);
}

